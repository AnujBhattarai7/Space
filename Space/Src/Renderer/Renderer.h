#pragma once

#include "Maths.h"
#include "Shaders.h"
#include "VulkanHeader.h"

namespace Space
{
    class VulkanApi;

    class Renderer
    {
    public:
        static Renderer &Get() { return *_Renderer; }

        static void Init(VulkanApiVersion _V);
        static void ShutDown();

        static void SetupRender();

        static void Render();

        static RenderApiType GetApiType() { return RenderApiType::VULKAN_1; }

        static void SetViewPort(const Vec2 &Size);
        static void SetClearColor(const Vec4 &Color);

    private:
        Renderer() {}

        static Renderer *_Renderer;

        VulkanApi* _VulkanApi;
    };
} // namespace Space
