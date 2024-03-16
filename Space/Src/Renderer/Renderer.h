#pragma once

#include "Maths.h"
#include "Shaders.h"
#include "VulkanHeader.h"
#include "VertexBuffers.h"

namespace Space
{
    class VulkanApi;
    class VertexBuffer;

    struct RenderData
    {
        // Store the Last Submitted Vertex Buffer
        const VertexBuffer *_VB = nullptr;
    };

    class Renderer
    {
    public:
        static Renderer &Get() { return *_Renderer; }

        static const RenderData &GetRenderData() { return _Renderer->_RenderData; }

        static void Init(VulkanApiVersion _V);
        static void ShutDown();

        static void SetupRender();

        static void Render();

        static RenderApiType GetApiType() { return RenderApiType::VULKAN_1; }

        static void SetViewPort(const Vec2 &Size);
        static void SetClearColor(const Vec4 &Color);
        static void SetFramesToRender(int n);
        static void OnWindowResized();

        static void Submit(const VertexBuffer &VB);

        static void Stop();
    private:
        Renderer() {}

        static Renderer *_Renderer;
        RenderData _RenderData{};

        VulkanApi *_VulkanApi;
    };
} // namespace Space
