#pragma once

#include "VulkanDevices.h"
#include "VulkanRenderChain.h"
#include "Maths.h"

namespace Space
{
    static std::vector<VkFramebuffer> _SwapChainFramebuffers;
    static std::vector<PhysicalDevice> _PhysicalDevices;

    static VulkanDevice _Device;

    struct VulkanApiData
    {
        VkInstance _VulkanInstance;
        VkDebugUtilsMessengerEXT _DebugMessenger;
        VkSurfaceKHR _Surface;

        VkRenderPass _RenderPass;
        VkPipelineLayout _PipelineLayout;
        VkPipeline _GraphicsPipeline;

        Vec2 _WindowSize;

        PhysicalDevice *_ActivePhysicalDevice = nullptr;
        int _ActivePhysicalDeviceIndex = -1;

        VulkanRenderChain _SwapChain;
    };

    struct VulkanApiRenderData
    {
        VkRenderPassBeginInfo _RenderPassBeginInfo{};
        Vec2 _ViewPortSize;
        VkExtent2D _ViewPortExtent;
        Vec4 _ClearColor;
        // Commands Buffers
        VkCommandPool _CommandPool;
        VkCommandBuffer _CommandBuffers;

        // Render
        VkSemaphore _Render, _ImageAvialable;
        VkFence _InFlight;
        VkRenderPass *_pRenderPass;
        VkPipeline *_pGraphicsPipeline;
        int _CuurentImageIndex = -1;
    };

    bool _IsPhysicalDeviceSuitable(VkPhysicalDevice _Device, VkSurfaceKHR _Surface);
    void _RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    class VulkanApi 
    {
    public:
        VulkanApi(VulkanApiVersion _V) { Init(_V); }
        ~VulkanApi() { ShutDown(); }

        bool Init(VulkanApiVersion _V);
        bool ShutDown();
        void Update();

        void SetupRender();
        void Render();

        virtual void SetViewPort(const Vec2 &Size);
        virtual void SetClearColor(const Vec4 &ClearColor) { _RenderData._ClearColor = ClearColor; }

    private:
        VulkanApiData _Data;
        VulkanApiRenderData _RenderData;

    private:
        void SetupFrameBuffers();
        void SetupInstance(VulkanApiVersion _V);
        void SetupDebugMessenger();
        void SetupSurface();
        void SetupActivePhysicalDevice();
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
