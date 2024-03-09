#include "PCH.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "VulkanApi.h"
#include "VulkanRender.h"
#include "VulkanDevices.h"
#include "VulkanSurfaceKHR.h"
#include "VulkanShader.h"

#include "WindowStack.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Space
{
    // ---- Static Variables ----
    std::vector<PhysicalDevice> _PhysicalDevices;

    // ---- Queues ----
    QueueFamilyIndices _FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR _Surface, const std::initializer_list<QueueFamilies> &_Families)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, nullptr);

        QueueFamilyIndices indices;

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(Device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            for (auto qf : _Families)
            {
                if (qf == QueueFamilies::GRAPHIC)
                {
                    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        indices.Families[(int)QueueFamilies::GRAPHIC] = i;
                }
                else if (qf == QueueFamilies::RENDER)
                    if (_Surface != VK_NULL_HANDLE)
                    {
                        VkBool32 presentSupport = false;
                        vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, _Surface, &presentSupport);

                        if (presentSupport)
                            indices.Families[(int)QueueFamilies::RENDER] = i;
                    }
                    else
                        SP_CORE_ERROR("Vulkan: Queury: _Surface Not Given")
            }

            i++;
        }

        return indices;
    }

    // ---- Physical Device ----

    void PhysicalDevice::SetupData()
    {
        VkPhysicalDeviceProperties DeviceProperties{};
        vkGetPhysicalDeviceProperties(_DeviceHandle, &DeviceProperties);

        // Store the Name
        while (DeviceProperties.deviceName[_Data._NameSize] != '\0')
        {
            _Data._Name[_Data._NameSize] = DeviceProperties.deviceName[_Data._NameSize];
            _Data._NameSize++;
        }

        _Data._DeviceID = DeviceProperties.deviceID;
        _Data._DriverVersion = DeviceProperties.driverVersion;
        _Data._VenderID = DeviceProperties.vendorID;
        _Data._DeviceType = (PhysicalDeviceType)DeviceProperties.deviceType;

        vkGetPhysicalDeviceFeatures(_DeviceHandle, &_Features);
    }

    bool PhysicalDevice::Init(VkPhysicalDevice _Device)
    {
        _DeviceHandle = _Device;
        SetupData();

        SP_CORE_PRINT("Vulkan: PhysicalDevice: ")
        _PrintDeviceInfo(_Data);

        return true;
    }

    std::vector<PhysicalDevice> &_GetPhysicalDevices()
    {
        return _PhysicalDevices;
    }

    bool _IsPhyscialDeviceSuitable(PhysicalDevice &_Devices, SurfaceKHR &_Surface, const std::vector<const char *> &_RequiredExtensions)
    {
        auto _Indicies = _FindQueueFamilies(_Devices, _Surface, {QueueFamilies::GRAPHIC, QueueFamilies::RENDER});
        // Check for SwapChain Support
        auto Details = _GetRenderchainSupport(_Devices, _Surface);
        // Check if the QueueFamilies has the features we or not

        // Checks if the device has the features we use or not
        return _Devices.GetData()._DeviceType == PHYSCIAL_DEVICE_TYPE_DISCRETE_GPU &&
               _Indicies._Exist() && _CheckPhysicalDeviceExetensions(_Devices, _RequiredExtensions) && Details._IsSuitable();
    }

    bool _CheckPhysicalDeviceExetensions(PhysicalDevice &_Devices, const std::vector<const char *> &_ExtensionNames)
    {
        uint32_t _ExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(_Devices, nullptr, &_ExtensionCount, nullptr);

        VkExtensionProperties _DeviceExtensions[_ExtensionCount];
        vkEnumerateDeviceExtensionProperties(_Devices, nullptr, &_ExtensionCount, _DeviceExtensions);

        int _ExistExtCount = _ExtensionNames.size();

        for (int j = 0; j < _ExtensionNames.size(); j++)
            for (int i = 0; i < _ExtensionCount; i++)
                if (strcmp(_ExtensionNames[j], _DeviceExtensions[i].extensionName) == 0)
                    _ExistExtCount--;

        // If there are less supported extensiosn than ExtensionCount then return false
        return _ExistExtCount == 0;
    }

    void _GetPhysicalDeviceExetensions(std::vector<const char *> &_Extensions, PhysicalDevice &_Devices)
    {
        uint32_t _ExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(_Devices, nullptr, &_ExtensionCount, nullptr);

        _Extensions.reserve(_ExtensionCount);

        VkExtensionProperties _DeviceExtensions[_ExtensionCount];
        vkEnumerateDeviceExtensionProperties(_Devices, nullptr, &_ExtensionCount, _DeviceExtensions);

        for (int i = 0; i < _ExtensionCount; i++)
            _Extensions[i] = _DeviceExtensions[i].extensionName;
    }

    uint32_t _GetPhysicalDeviceCount(VkInstance _Instance)
    {
        // Gets the Vulkan Supporting Devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_Instance, &deviceCount, nullptr);

        return deviceCount;
    }

    void _GetPhysicalDevices(VkInstance _Instance, PhysicalDevice *_Devices, int DeviceCount)
    {
        // TODO: Stores all of the Devices in an Array of Class
        if (DeviceCount == 0)
            SP_CORE_ERROR("Vulkan: Found no GPUs")

        VkPhysicalDevice DeviceHandles[DeviceCount];
        uint32_t _DC = DeviceCount;
        vkEnumeratePhysicalDevices(_Instance, &_DC, DeviceHandles);

        for (int i = 0; i < DeviceCount; i++)
            _Devices[i].Init(DeviceHandles[i]);
    }

    // ---- Device ----

    Device::~Device()
    {
        if (_Device != VK_NULL_HANDLE)
            SP_CORE_ERROR("Vulkan: Device: Destroy Not Called!!")
    }

    void Device::Init(PhysicalDevice &_PDevice, const SurfaceKHR &_Surface, ValidationLayersData &_ValidationLayers)
    {
        _PhysicalDeviceID = _PDevice.GetID();

        QueueFamilyIndices _Indicies = _FindQueueFamilies(_PDevice.GetDeviceHandle(), _Surface,
                                                          {QueueFamilies::GRAPHIC,
                                                           QueueFamilies::RENDER});
        // Sets up all of the Queues or Commands supported by the Physcial Device or GPU to the
        // The Logical Device Interface

        VkDeviceQueueCreateInfo queueCreateInfos[(int)QueueFamilies::SIZE];

        float queuepriorty = 1.0f;
        int i = 0;

        for (uint32_t queuefamily : _Indicies.Families)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queuefamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuepriorty;

            queueCreateInfos[i++] = queueCreateInfo;
        }

        VkPhysicalDeviceFeatures _DeviceFeatures{};

        VkDeviceCreateInfo _DeviceCreateInfo{};
        _DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        _DeviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
        _DeviceCreateInfo.queueCreateInfoCount = (int)QueueFamilies::SIZE;

        _DeviceCreateInfo.pEnabledFeatures = &_DeviceFeatures;
        _DeviceCreateInfo.enabledExtensionCount = _DeviceExtensions.size();
        _DeviceCreateInfo.ppEnabledExtensionNames = _DeviceExtensions.data();
        _DeviceCreateInfo.enabledLayerCount = 0;

        if (_ValidationLayers._Enable)
        {
            _DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(_ValidationLayers.Size());
            _DeviceCreateInfo.ppEnabledLayerNames = _ValidationLayers._Layers.data();
        }

        if (vkCreateDevice(_PDevice.GetDeviceHandle(), &_DeviceCreateInfo, nullptr, &_Device) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: failed to create logical device!");

        vkGetDeviceQueue(_Device, _Indicies.Families[(int)QueueFamilies::GRAPHIC], 0, &graphicsQueue);
        vkGetDeviceQueue(_Device, _Indicies.Families[(int)QueueFamilies::RENDER], 0, &_RenderQueue);

        SP_CORE_PRINT("Vulkan: Logical Device Interface created for: " << _PDevice.GetData()._Name)
    }

    void Device::Destroy()
    {
        vkDestroyDevice(_Device, nullptr);
        _Device = VK_NULL_HANDLE;
    }

    // ---- SurfaceKHR ----

    SurfaceKHR::~SurfaceKHR()
    {
        if (_Surface != VK_NULL_HANDLE)
            SP_CORE_ERROR("Vulkan: SurfaceKHR: Destroy Not Called!!")
    }

    void SurfaceKHR::Init(VkInstance _VKInstance)
    {
        if (glfwCreateWindowSurface(_VKInstance,
                                    (GLFWwindow *)WindowStack::GetWindows(0).GetWindow(),
                                    nullptr, &_Surface) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create WindowSurfaceKHR")
    }

    void SurfaceKHR::Destroy(VkInstance _VKInstance)
    {
        vkDestroySurfaceKHR(_VKInstance, _Surface, nullptr);
        _Surface = VK_NULL_HANDLE;
    }

    // ---- RenderChain

    RenderChain::~RenderChain()
    {
        if (_SwapChain != VK_NULL_HANDLE)
            SP_CORE_ERROR("Vulkan: RenderChain: Destroy not called")
    }

    void RenderChain::Init(VkPhysicalDevice _PDevice, VkSurfaceKHR _Surface, VkDevice _Device)
    {
        _InitSwapChainTarget(_PDevice, _Surface, _Device);
        _InitSwapChainImages(_Device);
    }

    void RenderChain::Destroy(VkDevice _Device)
    {
        for (auto &i : _RenderImageViews)
            vkDestroyImageView(_Device, i, nullptr);

        vkDestroySwapchainKHR(_Device, _SwapChain, nullptr);
        _SwapChain = VK_NULL_HANDLE;
    }

    RenderChainData _GetRenderchainSupport(VkPhysicalDevice _Device, VkSurfaceKHR _Surface)
    {
        RenderChainData _Details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_Device, _Surface, &_Details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_Device, _Surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            _Details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(_Device, _Surface, &formatCount, _Details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(_Device, _Surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            _Details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(_Device, _Surface, &presentModeCount, _Details.presentModes.data());
        }

        return _Details;
    }

    VkSurfaceFormatKHR _GetSuitableRenderSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &_Formats)
    {

        for (const auto &availableFormat : _Formats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;

        return _Formats[0];
    }

    VkPresentModeKHR _GetSuitableRenderSurfacePresentMode(const std::vector<VkPresentModeKHR> &_PresentModess)
    {
        for (const auto &availablePresentMode : _PresentModess)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D _GetSuitableRenderExtent(const VkSurfaceCapabilitiesKHR &_Capabilites)
    {
        if (_Capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return _Capabilites.currentExtent;
        else
        {
            auto FBSize = WindowStack::GetRenderWindow().GetFrameBufferSize();
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(FBSize.x),
                static_cast<uint32_t>(FBSize.y)};

            actualExtent.width = std::clamp(actualExtent.width, _Capabilites.minImageExtent.width, _Capabilites.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, _Capabilites.minImageExtent.height, _Capabilites.maxImageExtent.height);

            return actualExtent;
        }
    }

    void RenderChain::_InitSwapChainTarget(VkPhysicalDevice _PDevice, VkSurfaceKHR _Surface, VkDevice _Device)
    {
        RenderChainData swapChainSupport = _GetRenderchainSupport(_PDevice, _Surface);

        VkSurfaceFormatKHR surfaceFormat = _GetSuitableRenderSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = _GetSuitableRenderSurfacePresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = _GetSuitableRenderExtent(swapChainSupport.capabilities);

        uint32_t ImageCount = swapChainSupport.capabilities.minImageCount + 1;
        // IF the ImageCount is greater than maxImageCount then set it to the max Image Count
        if (ImageCount > swapChainSupport.capabilities.maxImageCount && swapChainSupport.capabilities.maxImageCount > 0)
            ImageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR _SwapChainInfo{};
        _SwapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        _SwapChainInfo.surface = _Surface;
        _SwapChainInfo.imageFormat = surfaceFormat.format;
        _SwapChainInfo.imageColorSpace = surfaceFormat.colorSpace;

        _SwapChainInfo.minImageCount = ImageCount;
        _SwapChainInfo.imageExtent = extent;
        _SwapChainInfo.imageArrayLayers = 1;
        _SwapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        _SwapChainInfo.presentMode = presentMode;
        _SwapChainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        _SwapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        _SwapChainInfo.clipped = true;
        _SwapChainInfo.oldSwapchain = VK_NULL_HANDLE;

        QueueFamilyIndices indices = _FindQueueFamilies(_PDevice, _Surface,
                                                        {QueueFamilies::GRAPHIC,
                                                         QueueFamilies::RENDER});

        // IF present in different queue families
        if (indices.Families[(int)QueueFamilies::GRAPHIC] != indices.Families[(int)QueueFamilies::RENDER])
        {
            _SwapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            _SwapChainInfo.queueFamilyIndexCount = 2;
            _SwapChainInfo.pQueueFamilyIndices = indices.Families;
        }
        else
        {
            _SwapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            _SwapChainInfo.queueFamilyIndexCount = 0;     // Optional
            _SwapChainInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        auto Result = vkCreateSwapchainKHR(_Device, &_SwapChainInfo, nullptr, &_SwapChain);

        if (Result != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Swap Chain Target Setup failed")
        SP_CORE_PRINT("Swap Chain Target Created!!\n")

        _SwapChainExtent = extent;
        _SwapChainImageFormat = surfaceFormat.format;
    }

    void RenderChain::_InitSwapChainImages(VkDevice _Device)
    {
        uint32_t _ImageCount = 0;
        vkGetSwapchainImagesKHR(_Device, _SwapChain, &_ImageCount, nullptr);
        _RenderImages.resize(_ImageCount);
        vkGetSwapchainImagesKHR(_Device, _SwapChain, &_ImageCount, _RenderImages.data());

        _RenderImageViews.resize(_RenderImages.size());

        for (int i = 0; i < _RenderImageViews.size(); i++)
        {
            VkImageViewCreateInfo _ImageViewInfo{};

            _ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            _ImageViewInfo.format = _SwapChainImageFormat;
            _ImageViewInfo.image = _RenderImages[i];
            _ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

            _ImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            _ImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            _ImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            _ImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            _ImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            _ImageViewInfo.subresourceRange.baseMipLevel = 0;
            _ImageViewInfo.subresourceRange.levelCount = 1;
            _ImageViewInfo.subresourceRange.baseArrayLayer = 0;
            _ImageViewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_Device, &_ImageViewInfo, nullptr, &_RenderImageViews[i]) != VK_SUCCESS)
                SP_CORE_ERROR("Vulkan: Image View Setup Failed")
        }
    }

    // ---- Funcs ----

    void _PrintDeviceInfo(const PhysicalDeviceData &Data)
    {
        SP_CORE_PRINT("GPU Info: ")
        SP_CORE_PRINT("Name:   " << Data._Name)
        SP_CORE_PRINT("Device ID:  " << Data._DeviceID)
        SP_CORE_PRINT("Driver Version:  " << Data._DriverVersion)
        SP_CORE_PRINT("Vendor ID:  " << Data._VenderID)

        if (Data._DeviceType == PHYSCIAL_DEVICE_TYPE_DISCRETE_GPU)
            SP_CORE_PRINT("GPU Type:     Discrete\n")
        if (Data._DeviceType == PHYSCIAL_DEVICE_TYPE_INEGRATED_GPU)
            SP_CORE_PRINT("GPU Type:     Integrated\n")
    }

}
