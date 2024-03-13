#include "PCH.h"
#include "Renderer.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "VulkanApi.h"

namespace Space
{
    void Renderer::Init(RenderApiType _RenderApiType)
    {
        SP_CORE_PRINT("Renderer Initialization: ")

        _Renderer->_ApiType = _RenderApiType;

        if (_RenderApiType == RenderApiType::VULKAN_1)
            _Renderer->_Api.Init(new VulkanApi());
        if (_RenderApiType == RenderApiType::VULKAN_2)
            _Renderer->_Api.Init(new VulkanApi());
        if (_RenderApiType == RenderApiType::OPENGL_3)
            // TODO:
            _Renderer->_Api.Init(new VulkanApi());
    }

    void Renderer::ShutDown()
    {
        SP_CORE_PRINT("Renderer ShutDown: ")
        _Renderer->_Api.ShutDown();
    }

    void Renderer::Begin(const Vec2 &_ViewPortSize)
    {
        _Renderer->_Api.Begin(_ViewPortSize);
    }

    void Renderer::End()
    {
        _Renderer->_Api.End();
    }

    void Renderer::Render()
    {
        _Renderer->_Api.Render();
    }

    RenderApiType GetActiveRenderApi()
    {
        return Renderer::GetApiType();
    }

} // namespace Space

// Create Funcs
namespace Space
{
    Shader *Shader::Create(const char *VertexFilePath, const char *ShaderFilePath)
    {
        SP_CORE_ERROR("Shader: Given Api Not Supoorted")
        return nullptr;
    }
}
