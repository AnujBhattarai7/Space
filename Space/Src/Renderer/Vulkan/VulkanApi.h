#pragma once

#include "Core.h"
#include "RenderApi.h"

#include "UUID.h"
#include "VulkanRender.h"
#include "VulkanDevices.h"

#include "VulkanShader.h"

namespace Space
{
    struct ValidationLayersData
    {
        bool _Enable = false;
        std::vector<const char *> _Layers;

        std::vector<const char *>::iterator begin() { return _Layers.begin(); }
        std::vector<const char *>::iterator end() { return _Layers.end(); }

        int Size() { return _Layers.size(); }

        ValidationLayersData()
        {
            _Layers.push_back("VK_LAYER_KHRONOS_validation");

            _Enable = true;
            if (GetBuildType() == BUILD_TYPE::RELEASE)
                _Enable = false;
        }
    };

    const std::vector<const char *> _DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // The Validation Layers Container
    static ValidationLayersData _ValidationLayers{};

    void _PrintDeviceInfo(const PhysicalDeviceData &Data);

    // Vulkan Used Functions
    bool _CheckValidationLayerSupport(ValidationLayersData &_Data);
    // Debug Msessenger Funcs
    void _DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
    VkResult _CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    void _SetupDebuggerMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    const Device& _GetActiveDevice();

    class VulkanApi : public RenderApi
    {
    public:
        VulkanApi()
        {
            Init();
        }
        ~VulkanApi() { ShutDown(); }

        // For Debugging all of the errors in the Vulkan
        void SetupDebugger();
        // Finds and Stores all of the Physical Device we found and from them find a suitable
        // GPU to use
        void SetupPhysicalDevice();
        // The interface we use to draw on the active PhysicalDevice
        void SetupLogicalDeviceInterface();

        void SetupSwapChain();
        void SetupImageViews();

        virtual bool Init() override;
        virtual bool ShutDown() override;

    private:
        // The Init Func for the main VulkanInstance
        bool _Init();

        // All of the Vk structs will be abstracted into Space Structs

        // The Raw Vulkan Instance
        VkInstance _VulkanInstance;
        // The Messenger which debugs the errors in the Vulkan Implementation
        VkDebugUtilsMessengerEXT debugMessenger;
        // The Logical Device Interface we use to render
        // Class : LogicalDevice
        Device _Device;

        // The Active physical Device(GPU) we render to
        PhysicalDevice *_PhysicalDevice = VK_NULL_HANDLE;
        int _ActivePhysicalDeviceIndex = -1;

        SurfaceKHR _Surface;
        RenderChain _RenderChain;
    };
} // namespace Space
