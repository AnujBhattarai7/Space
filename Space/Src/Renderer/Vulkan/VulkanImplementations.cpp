#include "PCH.h"

#include "WindowStack.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "VulkanDevices.h"
#include "VulkanRenderChain.h"

#include "Timer.h"

#include <GLFW/glfw3.h>

namespace Space
{
    // ---- Render Chain ----

    static std::vector<VkImageView> _SwapChainImageViews;

    void VulkanRenderChain::Init(VkSurfaceKHR _Surface, VulkanDevice &_Device)
    {
        auto Details = _GetSwapChainSupportDetails(_Surface, _Device.GetPhysicalDevice());

        auto Extent = _ChooseSwapExtent(Details.capabilities);
        auto Format = _ChooseSwapSurfaceFormat(Details.formats);
        auto PresentMode = _ChooseSwapPresentMode(Details.presentModes);

        _ImageCount = Details.capabilities.minImageCount + 1;
        if (Details.capabilities.maxImageCount > 0 && _ImageCount > Details.capabilities.maxImageCount)
            _ImageCount = Details.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR _CreateInfo{};
        _CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        _CreateInfo.presentMode = PresentMode;
        _CreateInfo.minImageCount = _ImageCount;

        _CreateInfo.imageExtent = Extent;
        _CreateInfo.imageFormat = Format.format;
        _CreateInfo.imageColorSpace = Format.colorSpace;
        _CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        _CreateInfo.imageArrayLayers = 1;
        _CreateInfo.surface = _Surface;

        QueueFamilyIndices indices = _Device.GetPhysicalDevice().GetQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = {indices._Indicies[GRAPHICS], indices._Indicies[RENDER],
                                         indices._Indicies[COMPUTE], indices._Indicies[TRANSFER]};
        _CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // If Render and Graphic queues present in diff queue families then allow transfer of data
        if (!indices.IsInSameFamily())
        {
            _CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            _CreateInfo.queueFamilyIndexCount = 2;
            _CreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }

        _CreateInfo.preTransform = Details.capabilities.currentTransform;
        _CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        _CreateInfo.presentMode = PresentMode;
        _CreateInfo.clipped = VK_TRUE;

        _CreateInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(_Device, &_CreateInfo, nullptr, &_SwapChain) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Swap Chain Target Creation Failed!!")
        SP_CORE_PRINT("Vulkan: Swap Chain Target Created")

        _Format = Format.format;
        _Extent = Extent;

        _InitImageViews(_Device);
    }

    void VulkanRenderChain::Destroy(VulkanDevice &_Device)
    {
        for (auto imageView : _SwapChainImageViews)
            vkDestroyImageView(_Device, imageView, nullptr);

        vkDestroySwapchainKHR(_Device, _SwapChain, nullptr);
    }

    void VulkanRenderChain::_InitImageViews(VulkanDevice &_Device)
    {
        VkImage _Images[_ImageCount];
        vkGetSwapchainImagesKHR(_Device, _SwapChain, &_ImageCount, _Images);
        _SwapChainImageViews.resize(_ImageCount);

        for (size_t i = 0; i < _ImageCount; i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = _Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = _Format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_Device, &createInfo, nullptr, &_SwapChainImageViews[i]) != VK_SUCCESS)
                SP_CORE_ERROR("Vulkan: Failed to create image views!");
        }

        SP_CORE_PRINT("Vulkan: Swap Chain Image Views Created!!\n")
    }

    const std::vector<VkImageView> &VulkanRenderChain::GetImageViews() const
    {
        // TODO: insert return statement here
        return _SwapChainImageViews;
    }

    // Physical Devices

    void PhysicalDevice::Init(VkPhysicalDevice Device)
    {
        _Device = Device;
        _Data = _SetupPhysicalDeviceData(_Device);
    }

    PhysicalDeviceData _SetupPhysicalDeviceData(VkPhysicalDevice _Device)
    {
        VkPhysicalDeviceProperties _Properties;
        vkGetPhysicalDeviceProperties(_Device, &_Properties);

        PhysicalDeviceData _Data;

        for (size_t i = 0; i < strlen(_Properties.deviceName); i++)
            _Data._Name[i] = _Properties.deviceName[i];

        _Data._ApiVersion = _Properties.apiVersion;
        _Data._VendorID = _Properties.vendorID;
        _Data._DeviceID = _Properties.deviceID;
        _Data._DriverVersion = _Properties.driverVersion;

        if (_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            _Data._DeviceType = INTEGRATED_GPU;
        else if (_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            _Data._DeviceType = DISCRETE_GPU;
        else if (_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
            _Data._DeviceType = CPU;
        else if (_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            _Data._DeviceType = VIRTUAL_GPU;

        return _Data;
    }

    // ---- Device ----
    void VulkanDevice::Init(PhysicalDevice &PDevice)
    {
        _PDevice = &PDevice;
        auto _Indices = PDevice.GetQueueFamilyIndices();

        VkDeviceCreateInfo _DeviceCreateInfo{};
        _DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        // Setup Queue Create Info
        std::vector<VkDeviceQueueCreateInfo> _QueueCreateInfos;
        float QueuePriorities = 1.0f;

        // Using Set because Vulkan Device requires unique family index for each queue create info so if we have same queue family index
        // then dont count it

        std::set<uint32_t> _Index;
        for (int i = 0; i < SIZE; i++)
            _Index.insert(_Indices._Indicies[i]);

        for (uint32_t i : _Index)
        {
            VkDeviceQueueCreateInfo _CreateInfo{};
            _CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            _CreateInfo.pQueuePriorities = &QueuePriorities;
            _CreateInfo.queueCount = 1;
            _CreateInfo.queueFamilyIndex = i;
            _QueueCreateInfos.push_back(_CreateInfo);
        }

        _DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(_QueueCreateInfos.size());
        _DeviceCreateInfo.pQueueCreateInfos = _QueueCreateInfos.data();

        VkPhysicalDeviceFeatures _F{};

        _DeviceCreateInfo.pEnabledFeatures = &_F;

        _DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        _DeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        _DeviceCreateInfo.enabledLayerCount = 0;

        if (GetBuildType() == BUILD_TYPE::DEBUG)
        {
            _DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            _DeviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }

        if (vkCreateDevice(PDevice.GetHandle(), &_DeviceCreateInfo, nullptr, &_Device) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Device Creation Failed!")

        for (int i = 0; i < SIZE; i++)
            vkGetDeviceQueue(_Device, _Indices._Indicies[i], 0, &_Queues[i]);

        SP_CORE_PRINT("Vulkan: Logical Device Created from: " << PDevice.GetData()._Name)
    }

    VulkanDevice::~VulkanDevice()
    {
    }

    void VulkanDevice::Destroy()
    {
        vkDestroyDevice(_Device, nullptr);
        _Device = VK_NULL_HANDLE;
        _PDevice = nullptr;
    }

    // Queues
    QueueFamilyIndices _GetDeviceQueueFamilies(VkPhysicalDevice _Device, VkSurfaceKHR _Surface)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_Device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_Device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices._Indicies[GRAPHICS] = i;
            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
                indices._Indicies[COMPUTE] = i;
            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
                indices._Indicies[TRANSFER] = i;

            // Find if the QueueFamily has The Queue used to present images
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(_Device, i, _Surface, &presentSupport);

            if (presentSupport)
                indices._Indicies[RENDER] = i;

            if (indices.isComplete())
                break;

            i++;
        }

        return indices;
    }

    // Raw Funcs
    void _PrintPhysicalDeviceInfo(const PhysicalDeviceData &_Device)
    {
        SP_CORE_PRINT("GPU Info: " << _Device._Name)
        SP_CORE_PRINT("Api Version: " << _Device._ApiVersion)
        SP_CORE_PRINT("Driver Version: " << _Device._DriverVersion)
        SP_CORE_PRINT("Device ID: " << _Device._DeviceID)
        SP_CORE_PRINT("Vendor ID: " << _Device._VendorID)

        if (_Device._DeviceType == INTEGRATED_GPU)
            SP_CORE_PRINT("Device Type: Integrated\n")
        else if (_Device._DeviceType == DISCRETE_GPU)
            SP_CORE_PRINT("Device Type: Discrete\n")
        else if (_Device._DeviceType == VIRTUAL_GPU)
            SP_CORE_PRINT("Device Type: Virtual\n")
    }

    VkSurfaceFormatKHR _ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR _ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    RenderChainSupportDetails _GetSwapChainSupportDetails(VkSurfaceKHR _Surface, VkPhysicalDevice device)
    {
        RenderChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _Surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _Surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _Surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _Surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _Surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkExtent2D _ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize((GLFWwindow *)WindowStack::GetRenderWindow().GetWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

}
