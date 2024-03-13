#pragma once

#include "RenderApi.h"

namespace Space
{
    enum QueuFamilyIndex
    {
        PRESENT,
        RENDER = 0,
        GRAPHICS,
        SIZE
    };

    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#if (SP_DEBUG)
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

    struct QueueFamilyIndices
    {
        uint32_t _Indicies[QueuFamilyIndex::SIZE];

        QueueFamilyIndices()
        {
            for (auto &i : _Indicies)
                i = 900000000000;
        }

        bool isComplete()
        {
            for (auto &i : _Indicies)
                if (i == 900000000000)
                    return false;
            return true;
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    static std::vector<VkImageView> _SwapChainImageViews;
    static std::vector<VkFramebuffer> _SwapChainFramebuffers;

    struct VulkanApiData
    {
        VkInstance _VulkanInstance;
        VkDebugUtilsMessengerEXT _DebugMessenger;
        VkSurfaceKHR _Surface;

        VkPhysicalDevice _PhysicalDevice = VK_NULL_HANDLE;
        int _ActivePhysicalDevice = -1;
        VkDevice _Device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR _SwapChain;
        uint32_t _SwapChainImageCount = 0;
        VkFormat _SwapChainImageFormat;
        VkExtent2D _SwapChainExtent;

        VkRenderPass _RenderPass;
        VkPipelineLayout _PipelineLayout;
        VkPipeline _GraphicsPipeline;

        Vec2 _WindowSize;

        // Commands Buffers
        VkCommandPool _CommandPool;
        VkCommandBuffer _CommandBuffers;

        // Render
        VkSemaphore _Render, _ImageAvialable;
        VkFence _InFlight;
    };

    void _PrintPhysicalDeviceInfo(VkPhysicalDevice _Device);
    bool _IsPhysicalDeviceSuitable(VkPhysicalDevice _Device, VkSurfaceKHR _Surface);
    void _RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    class VulkanApi : public RenderApi
    {
    public:
        VulkanApi() { Init(); }
        ~VulkanApi() { ShutDown(); }

        bool Init() override;
        bool ShutDown() override;
        void Update() override;

        void Begin(const Vec2 &_VWSize) override;
        void End() override;
        void Render() override;

    private:
        VulkanApiData _Data;

    private:
        void SetupFrameBuffers();
        void SetupInstance();
        void SetupDebugMessenger();
        void SetupSurface();
        void SetupActivePhysicalDevice();
        void SetupLogicalDevice();
        void SetupSwapChain();
        void SetupImageViews();
        void SetupRenderPass();
        void SetupGraphicPipeline();
        void SetupCommandPool();
        void SetupCommandBuffers();
        void SetupSyncObjects();

        VkShaderModule CreateShaderModule(const std::vector<char> &code);
    };
} // namespace Space
