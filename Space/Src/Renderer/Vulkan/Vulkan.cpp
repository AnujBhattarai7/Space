#include "PCH.h"
// For Profiling
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "VulkanApi.h"

#include "Timer.h"
// For Getting the GLFW required extensions
#include "Window.h"

#include "WindowStack.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Space
{
    static Device* _ActiveDevice;
    // Raw Funcs
    bool _CheckValidationLayerSupport(ValidationLayersData &_Data)
    {
        if (!_Data._Enable)
            SP_CORE_ERROR("Vulkan: Validation Layers Check called even when disabled")

        // Number of layers in the instance
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        // Stores the Available layers
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : _Data)
        {
            bool layerFound = false;
            // Checks if the Layers we added are in the available layers or not
            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return false;
        }

        SP_CORE_PRINT("Vulkan: Using Validataion Layers")
        return true;
    }

    void _DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    VkResult _CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void _SetupDebuggerMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    const Device &_GetActiveDevice()
    {
        return *_ActiveDevice;
    }

    // VulkanApi Funcs
    void VulkanApi::SetupDebugger()
    {
        if (!_ValidationLayers._Enable)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        _SetupDebuggerMessengerInfo(createInfo);

        if (_CreateDebugUtilsMessengerEXT(_VulkanInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void VulkanApi::SetupPhysicalDevice()
    {
        // Gets the Vulkan Supporting Devices
        uint32_t deviceCount = _GetPhysicalDeviceCount(_VulkanInstance);

        if (deviceCount == 0)
            SP_CORE_ERROR("Vulkan: Found no GPUs supporting Vuilkan")

        SP_CORE_PRINT("Vulkan: " << deviceCount << " Physical Devices Detected: \n")

        _GetPhysicalDevices().resize(deviceCount);
        _GetPhysicalDevices(_VulkanInstance, _GetPhysicalDevices().data(), deviceCount);

        // Set the VulkanApi Active PhysicalDevice according to the enabled features
        for (int i = 0; i < deviceCount; i++)
        {
            if (_IsPhyscialDeviceSuitable(_GetPhysicalDevices()[i], _Surface, _DeviceExtensions))
            {
                _ActivePhysicalDeviceIndex = i;
                // Points to the Physical Device we chooese to use
                _PhysicalDevice = &_GetPhysicalDevices()[i];
                break;
            }
        }

        // Chech if No suitable GPU found then errror
        if (_PhysicalDevice->GetDeviceHandle() == VK_NULL_HANDLE)
            SP_CORE_ERROR("Vulkan: No Suitable GPU found")

        // If suitable GPU found then print its data
        SP_CORE_PRINT("The Active GPU:  ")
        _PrintDeviceInfo(_PhysicalDevice->GetData());
    }

    bool VulkanApi::Init()
    {
        SP_PROFILE_SCOPE("Vulkan Initialization: ")
        // The Init of the Vulkan Instance
        _Init();
        // Setup the debugger for validation layers
        SetupDebugger();
        // TODO: Be abstracted into the Vulkan Window Specification
        _Surface.Init(_VulkanInstance);
        // Setup all of the physical device
        SetupPhysicalDevice();
        // Setup the Physcial device logical device interface which we use in rendering
        _Device.Init(_GetPhysicalDevices()[_ActivePhysicalDeviceIndex], _Surface, _ValidationLayers);
        _ActiveDevice = &_Device;
        _RenderChain.Init(_PhysicalDevice->GetDeviceHandle(), _Surface.GetHandle(), _Device);

        return true;
    }

    bool VulkanApi::ShutDown()
    {
        if (_VulkanInstance == VK_NULL_HANDLE)
            return true;

        if (_ValidationLayers._Enable)
            _DestroyDebugUtilsMessengerEXT(_VulkanInstance, debugMessenger, nullptr);
        
        _RenderChain.Destroy(_Device);
        _Device.Destroy();
        _ActiveDevice = nullptr;

        _Surface.Destroy(_VulkanInstance);
        vkDestroyInstance(_VulkanInstance, nullptr);
        
        _VulkanInstance = VK_NULL_HANDLE;
        SP_CORE_PRINT("Vulkan Instance Destroyed\n")

        return true;
    }

    bool VulkanApi::_Init()
    {
        SP_PROFILE_SCOPE("Vulkan Instance Creation!!")

        if (_ValidationLayers._Enable && !_CheckValidationLayerSupport(_ValidationLayers))
            SP_CORE_ERROR("Vulkan: validation layers requested, but not available!")

        VkApplicationInfo _Info{};
        _Info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        _Info.pApplicationName = "Space";
        _Info.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
        _Info.pEngineName = "No Engine";
        _Info.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
        // Sets the Api version to use
        _Info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo _In_Info{};
        // Set the Struct Type
        _In_Info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        _In_Info.pApplicationInfo = &_Info;

        SP_CORE_PRINT("Using Vulkan For Rendering!!")
        std::vector<const char *> _Extensions;

        SetRequiredExtensions(_Extensions);
        _In_Info.enabledExtensionCount = _Extensions.size();
        _In_Info.ppEnabledExtensionNames = _Extensions.data();

        _In_Info.enabledLayerCount = 0;
        _In_Info.pNext = nullptr;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        if (_ValidationLayers._Enable)
        {
            _In_Info.enabledLayerCount = _ValidationLayers._Layers.size();
            _In_Info.ppEnabledLayerNames = _ValidationLayers._Layers.data();

            _SetupDebuggerMessengerInfo(debugCreateInfo);

            _In_Info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }

        VkResult result = vkCreateInstance(&_In_Info, nullptr, &_VulkanInstance);

        if (result != VK_SUCCESS)
            SP_CORE_ERROR("Vulkan Instance Setup Failed!!")
        SP_CORE_PRINT("Vulkan Instance Created\n");

        return true;
    }
} // namespace Space
