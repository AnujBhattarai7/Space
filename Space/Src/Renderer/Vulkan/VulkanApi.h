#pragma once

#include "VulkanDevices.h"
#include "VulkanRenderChain.h"
#include "VulkanRenderPass.h"

#include "Maths.h"

namespace Space
{
    class VertexBuffer;

    struct VulkanApiData
    {
        VkInstance _VulkanInstance;
        VkDebugUtilsMessengerEXT _DebugMessenger;
        VkSurfaceKHR _Surface;

        // VkRenderPass _RenderPass;

        VulkanRenderPass _RenderPass;
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
        std::vector<VkCommandBuffer> _CommandBuffers;

        // Render
        std::vector<VkSemaphore> _Render, _ImageAvialable;
        std::vector<VkFence> _InFlight;
        VulkanRenderPass *_pRenderPass;
        VkPipeline *_pGraphicsPipeline;
        int _CuurentImageIndex = -1;

        // Stores the Number of Frames to Render at a time
        int _FramesNumber = 2;

        bool _FrameBufferRecreate = false;
        const VertexBuffer* _pCurrentVBuffer;
        // VkDeviceMemory _VBO_Data;
    };

    bool _IsPhysicalDeviceSuitable(VkPhysicalDevice _Device, VkSurfaceKHR _Surface);
    void _RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    const VulkanDevice& GetActiveDevice();

    class VulkanApi
    {
    public:
        VulkanApi(VulkanApiVersion _V) { Init(_V); }
        ~VulkanApi() { ShutDown(); }

        bool Init(VulkanApiVersion _V);
        bool ShutDown();
        void Update();

        void SetupRender();
        void Render(const VertexBuffer* _VB);
        void OnWindowResized() { _RenderData._FrameBufferRecreate = true; }

        void SetViewPort(const Vec2 &Size);
        void SetClearColor(const Vec4 &ClearColor);
        void SetFramesToRender(int n);
        void Stop();

    private:
        VulkanApiData _Data;
        VulkanApiRenderData _RenderData;

    private:
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        void SetupInstance(VulkanApiVersion _V);
        void SetupDebugMessenger();
        void SetupSurface();
        void SetupActivePhysicalDevice();
        void SetupGraphicPipeline();
        void SetupCommandPool();
        void SetupCommandBuffers();
        void SetupSyncObjects();

        VkShaderModule CreateShaderModule(const std::vector<char> &code);
    };
} // namespace Space
