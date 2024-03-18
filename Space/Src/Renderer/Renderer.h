#pragma once

#include "Maths.h"
#include "Shaders.h"
#include "VulkanHeader.h"
#include "VertexArray.h"

namespace Space
{
    class VulkanApi;
    class VertexBuffer;

    struct RenderData
    {
        // Store the Last Submitted Vertex Buffer
        const VertexBuffer *_VB = nullptr;
        const IndexBuffer *_IB = nullptr;
    };

    class Renderer
    {
    public:
        static Renderer &Get() { return *_Renderer; }

        static const RenderData &GetRenderData() { return _Renderer->_RenderData; }

        static void Init(VulkanApiVersion _V);
        static void ShutDown();

        static void SetupRender();

        static void Render(const VertexArray* VAO);

        static RenderApiType GetApiType() { return RenderApiType::VULKAN_1; }

        static void SetViewPort(const Vec2 &Size);
        static void SetClearColor(const Vec4 &Color);
        static void SetFramesToRender(int n);
        static void OnWindowResized();

        static void Stop();
    private:
        Renderer() {}

        static Renderer *_Renderer;
        RenderData _RenderData{};

        VulkanApi *_VulkanApi;
    };
} // namespace Space
