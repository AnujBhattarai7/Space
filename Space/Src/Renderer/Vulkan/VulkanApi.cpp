#include "PCH.h"

#include "WindowStack.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "VulkanDevices.h"

#include "Timer.h"
#include "VulkanApi.h"

#include <GLFW/glfw3.h>

namespace Space
{
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
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

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR _Surface, VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

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

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
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

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice _Device, VkSurfaceKHR _Surface)
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

    bool _IsPhysicalDeviceSuitable(VkPhysicalDevice _Device, VkSurfaceKHR _Surface)
    {
        QueueFamilyIndices indices = findQueueFamilies(_Device, _Surface);

        bool extensionsSupported = checkDeviceExtensionSupport(_Device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_Surface, _Device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    void _RecordCommandBuffer(const VulkanApiData &_Data, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;                  // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(_Data._CommandBuffers, &beginInfo) != VK_SUCCESS)
            SP_CORE_ERROR("failed to begin recording command buffer!");

        VkRenderPassBeginInfo _BeginInfo{};
        _BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        _BeginInfo.renderPass = _Data._RenderPass;
        _BeginInfo.framebuffer = _SwapChainFramebuffers[imageIndex];

        _BeginInfo.renderArea.extent = _Data._SwapChainExtent;
        _BeginInfo.renderArea.offset = {0, 0};

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        _BeginInfo.clearValueCount = 1;
        _BeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(_Data._CommandBuffers, &_BeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(_Data._CommandBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS, _Data._GraphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_Data._SwapChainExtent.width);
        viewport.height = static_cast<float>(_Data._SwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_Data._CommandBuffers, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = _Data._SwapChainExtent;
        vkCmdSetScissor(_Data._CommandBuffers, 0, 1, &scissor);

        vkCmdDraw(_Data._CommandBuffers, 3, 1, 0, 0);

        vkCmdEndRenderPass(_Data._CommandBuffers);

        if (vkEndCommandBuffer(_Data._CommandBuffers) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to record command buffer!");
    }

    std::vector<const char *> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    static std::vector<char> readFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            SP_CORE_ERROR("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    VkShaderModule VulkanApi::CreateShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_Data._Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    bool VulkanApi::Init()
    {
        SP_PROFILE_SCOPE("Vulkan: Initialization: ")
        SetupInstance();
        SetupDebugMessenger();
        SetupSurface();
        SetupActivePhysicalDevice();
        SetupLogicalDevice();
        SetupSwapChain();
        SetupImageViews();
        SetupRenderPass();
        SetupGraphicPipeline();
        SetupFrameBuffers();
        SetupCommandPool();
        SetupCommandBuffers();
        SetupSyncObjects();

        return true;
    }

    bool VulkanApi::ShutDown()
    {
        SP_PROFILE_SCOPE("Vulkan: ShutDown")

        for (auto framebuffer : _SwapChainFramebuffers)
            vkDestroyFramebuffer(_Data._Device, framebuffer, nullptr);

        vkDestroySemaphore(_Data._Device, _Data._Render, nullptr);
        vkDestroySemaphore(_Data._Device, _Data._ImageAvialable, nullptr);
        vkDestroyFence(_Data._Device, _Data._InFlight, nullptr);

        vkDestroyPipeline(_Data._Device, _Data._GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_Data._Device, _Data._PipelineLayout, nullptr);
        vkDestroyRenderPass(_Data._Device, _Data._RenderPass, nullptr);

        for (auto imageView : _SwapChainImageViews)
            vkDestroyImageView(_Data._Device, imageView, nullptr);

        vkFreeCommandBuffers(_Data._Device, _Data._CommandPool, 1, &_Data._CommandBuffers);
        vkDestroyCommandPool(_Data._Device, _Data._CommandPool, nullptr);

        vkDestroySwapchainKHR(_Data._Device, _Data._SwapChain, nullptr);
        vkDestroyDevice(_Data._Device, nullptr);

        if (enableValidationLayers)
            DestroyDebugUtilsMessengerEXT(_Data._VulkanInstance, _Data._DebugMessenger, nullptr);

        vkDestroySurfaceKHR(_Data._VulkanInstance, _Data._Surface, nullptr);
        vkDestroyInstance(_Data._VulkanInstance, nullptr);
        _Data._VulkanInstance = VK_NULL_HANDLE;

        return _Data._VulkanInstance == VK_NULL_HANDLE;
    }

    void VulkanApi::Update()
    {
    }

    void VulkanApi::Begin(const Vec2 &_VWSize)
    {
    }

    void VulkanApi::End()
    {
    }

    void VulkanApi::Render()
    {
        // Wait for PRevious frame to be finished
        vkWaitForFences(_Data._Device, 1, &_Data._InFlight, VK_TRUE, UINT64_MAX);
        vkResetFences(_Data._Device, 1, &_Data._InFlight);

        uint32_t _ImageIndex = 0;
        vkAcquireNextImageKHR(_Data._Device, _Data._SwapChain, UINT64_MAX, _Data._ImageAvialable, VK_NULL_HANDLE, &_ImageIndex);

        vkResetCommandBuffer(_Data._CommandBuffers, 0);

        _RecordCommandBuffer(_Data, _ImageIndex);

        VkSemaphore waitSemaphores[] = {_Data._ImageAvialable};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo _SubmitInfo{};
        _SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        _SubmitInfo.commandBufferCount = 1;
        _SubmitInfo.pCommandBuffers = &_Data._CommandBuffers;

        _SubmitInfo.pWaitDstStageMask = waitStages;
        _SubmitInfo.pWaitSemaphores = waitSemaphores;
        _SubmitInfo.waitSemaphoreCount = 1;

        VkSemaphore signalSemaphores[] = {_Data._Render};
        _SubmitInfo.signalSemaphoreCount = 1;
        _SubmitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_Data.graphicsQueue, 1, &_SubmitInfo, _Data._InFlight) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to submit draw command buffer!");

        VkPresentInfoKHR _PresentInfo{};
        _PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        _PresentInfo.waitSemaphoreCount = 1;
        _PresentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {_Data._SwapChain};

        _PresentInfo.swapchainCount = 1;
        _PresentInfo.pSwapchains = swapChains;
        _PresentInfo.pImageIndices = &_ImageIndex;
        _PresentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(_Data.presentQueue, &_PresentInfo);
    }

    void VulkanApi::SetupFrameBuffers()
    {
        _SwapChainFramebuffers.resize(_Data._SwapChainImageCount);

        for (size_t i = 0; i < _Data._SwapChainImageCount; i++)
        {
            VkImageView attachments[] = {
                _SwapChainImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = _Data._RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = _Data._SwapChainExtent.width;
            framebufferInfo.height = _Data._SwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(_Data._Device, &framebufferInfo, nullptr, &_SwapChainFramebuffers[i]) != VK_SUCCESS)
                SP_CORE_ERROR("Vulkan: Failed to create Framebuffer!");
        }
        SP_CORE_PRINT("Vulkan: FrameBuffers Setup!!")
    }

    void VulkanApi::SetupInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport())
            SP_CORE_ERROR("Vulkan: validation layers requested, but not available!");

        uint32_t _ExtCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &_ExtCount, nullptr);
        VkExtensionProperties _Prop[_ExtCount];
        vkEnumerateInstanceExtensionProperties(nullptr, &_ExtCount, _Prop);

        SP_CORE_PRINT("Vulkan: " << _ExtCount << " Extensions Supported!")
        SP_CORE_PRINT("Vulkan: Supported Extensions: ")
        for (auto &i : _Prop)
            SP_CORE_PRINT(i.extensionName)

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &_Data._VulkanInstance) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create instance!");
    }

    void VulkanApi::SetupDebugMessenger()
    {
        if (!enableValidationLayers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(_Data._VulkanInstance, &createInfo, nullptr, &_Data._DebugMessenger) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to set up debug messenger!");

        SP_CORE_PRINT("Vulkan: Setup Debug Mesenger")
    }

    void VulkanApi::SetupSurface()
    {
        if (glfwCreateWindowSurface(_Data._VulkanInstance, (GLFWwindow *)WindowStack::GetRenderWindow().GetWindow(), nullptr, &_Data._Surface) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create window surface!");
    }

    void VulkanApi::SetupActivePhysicalDevice()
    {
        uint32_t DeviceCount = 0;
        vkEnumeratePhysicalDevices(_Data._VulkanInstance, &DeviceCount, nullptr);

        if (DeviceCount == 0)
            SP_CORE_ERROR("Vulkan: 0 Physical Devices Found!!")

        SP_CORE_PRINT(DeviceCount << " Physical Devices Found\n")

        VkPhysicalDevice _Devices[DeviceCount];
        vkEnumeratePhysicalDevices(_Data._VulkanInstance, &DeviceCount, _Devices);

        for (auto &i : _Devices)
            _PrintPhysicalDeviceInfo(i);

        for (auto &i : _Devices)
        {
            if (_IsPhysicalDeviceSuitable(i, _Data._Surface))
            {
                _Data._PhysicalDevice = i;
                break;
            }
        }

        if (_Data._PhysicalDevice == VK_NULL_HANDLE)
            SP_CORE_PRINT("No Proper Vulkan Supported Physical Device FounD!!")

        SP_CORE_PRINT("Active Physical Device: ")
        _PrintPhysicalDeviceInfo(_Data._PhysicalDevice);
    }

    void VulkanApi::SetupLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(_Data._PhysicalDevice, _Data._Surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices._Indicies[GRAPHICS], indices._Indicies[RENDER]};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.enabledLayerCount = 0;

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }

        if (vkCreateDevice(_Data._PhysicalDevice, &createInfo, nullptr, &_Data._Device) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create logical device!");

        vkGetDeviceQueue(_Data._Device, indices._Indicies[GRAPHICS], 0, &_Data.graphicsQueue);
        vkGetDeviceQueue(_Data._Device, indices._Indicies[RENDER], 0, &_Data.presentQueue);

        SP_CORE_PRINT("Vulkan: Logical Device Created!!")
    }

    void VulkanApi::SetupSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_Data._Surface, _Data._PhysicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        _Data._SwapChainImageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && _Data._SwapChainImageCount > swapChainSupport.capabilities.maxImageCount)
            _Data._SwapChainImageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = _Data._Surface;

        createInfo.minImageCount = _Data._SwapChainImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(_Data._PhysicalDevice, _Data._Surface);
        uint32_t queueFamilyIndices[] = {indices._Indicies[GRAPHICS], indices._Indicies[RENDER]};
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // If Render and Graphic queues presentin diff queue families then allow transfer of data
        if (indices._Indicies[RENDER] != indices._Indicies[GRAPHICS])
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(_Data._Device, &createInfo, nullptr, &_Data._SwapChain) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create swap chain!");

        SP_CORE_PRINT("Vulkan: Swap Chain Target Created!!")

        _Data._SwapChainImageFormat = surfaceFormat.format;
        _Data._SwapChainExtent = extent;
    }

    void VulkanApi::SetupImageViews()
    {
        VkImage _Images[_Data._SwapChainImageCount];
        vkGetSwapchainImagesKHR(_Data._Device, _Data._SwapChain, &_Data._SwapChainImageCount, _Images);
        _SwapChainImageViews.resize(_Data._SwapChainImageCount);

        for (size_t i = 0; i < _Data._SwapChainImageCount; i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = _Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = _Data._SwapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_Data._Device, &createInfo, nullptr, &_SwapChainImageViews[i]) != VK_SUCCESS)
                SP_CORE_ERROR("Vulkan: Failed to create image views!");
        }

        SP_CORE_PRINT("Vulkan: Swap Chain Image Views Created!!\n")
    }

    void VulkanApi::SetupRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = _Data._SwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(_Data._Device, &renderPassInfo, nullptr, &_Data._RenderPass) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create render pass!\n");
        SP_CORE_PRINT("Vulkan: Render Pass Created!!\n")
    }

    void VulkanApi::SetupGraphicPipeline()
    {
        SP_PROFILE_SCOPE("Vulkan: Graphics Pipeline")

        auto vertShaderCode = readFile("../../../../Assets/Vert.spv");
        auto fragShaderCode = readFile("../../../../Assets/Frag.spv");

        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        // Shader Stage Info
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        // Create Shader Stages
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(_Data._Device, &pipelineLayoutInfo, nullptr, &_Data._PipelineLayout) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = _Data._PipelineLayout;
        pipelineInfo.renderPass = _Data._RenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(_Data._Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_Data._GraphicsPipeline) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Failed to create Graphics Pipeline!");

        vkDestroyShaderModule(_Data._Device, fragShaderModule, nullptr);
        vkDestroyShaderModule(_Data._Device, vertShaderModule, nullptr);
    }

    void VulkanApi::SetupCommandPool()
    {
        auto _Families = findQueueFamilies(_Data._PhysicalDevice, _Data._Surface);

        VkCommandPoolCreateInfo _CreateInfo{};
        _CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        _CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        _CreateInfo.queueFamilyIndex = _Families._Indicies[GRAPHICS];

        if (vkCreateCommandPool(_Data._Device, &_CreateInfo, nullptr, &_Data._CommandPool) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Command Pool Setup Failed!!")
        SP_CORE_PRINT("Vulkan: Command Pool Created!!\n")
    }

    void VulkanApi::SetupCommandBuffers()
    {
        VkCommandBufferAllocateInfo _Info{};
        _Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        _Info.commandPool = _Data._CommandPool;
        _Info.commandBufferCount = 1;
        _Info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(_Data._Device, &_Info, &_Data._CommandBuffers) != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan: Command Buffer Setup Failed!!")
        SP_CORE_PRINT("Vulkan: Command Buffer Created!!\n")
    }

    void VulkanApi::SetupSyncObjects()
    {
        VkSemaphoreCreateInfo _SemInfo{};
        _SemInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo _FenInfo{};
        _FenInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        _FenInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(_Data._Device, &_SemInfo, nullptr, &_Data._ImageAvialable) != VK_SUCCESS ||
            vkCreateSemaphore(_Data._Device, &_SemInfo, nullptr, &_Data._Render) != VK_SUCCESS ||
            vkCreateFence(_Data._Device, &_FenInfo, nullptr, &_Data._InFlight) != VK_SUCCESS)
            SP_CORE_ERROR("failed to create semaphores!");
    }

    void _PrintPhysicalDeviceInfo(VkPhysicalDevice _Device)
    {
        VkPhysicalDeviceProperties _Properties;
        vkGetPhysicalDeviceProperties(_Device, &_Properties);

        SP_CORE_PRINT("GPU Info: " << _Properties.deviceName)
        SP_CORE_PRINT("Api Version: " << _Properties.apiVersion)
        SP_CORE_PRINT("Driver Version: " << _Properties.driverVersion)
        SP_CORE_PRINT("Device ID: " << _Properties.deviceID)
        SP_CORE_PRINT("Vendor ID: " << _Properties.vendorID)

        if (_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            SP_CORE_PRINT("Device Type: Integrated\n")
        if (_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            SP_CORE_PRINT("Device Type: Discrete\n")
        if (_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            SP_CORE_PRINT("Device Type: Virtual\n")
    }

}
