#include "PCH.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "Renderer.h"
#include "VulkanApi.h"

namespace Space
{
    void Renderer::Init(VulkanApiVersion _V)
    {
        SP_CORE_PRINT("Renderer Initialization: ")

        _Renderer->_VulkanApi = new VulkanApi(_V);
    }

    void Renderer::ShutDown()
    {
        SP_CORE_PRINT("Renderer ShutDown: ")
        _Renderer->_VulkanApi->ShutDown();
    }

    void Renderer::SetupRender()
    {
        _Renderer->_VulkanApi->SetupRender();
        _Renderer->_VulkanApi->SetViewPort({1200, 900});
        _Renderer->_VulkanApi->SetClearColor({0.2f, 0.2f, 0.2f, 1.0f});
    }

    void Renderer::Render()
    {
        _Renderer->_VulkanApi->Render(_Renderer->_RenderData._VB);
    }

    void Renderer::SetViewPort(const Vec2 &Size)
    {
        if (Size.x == 0 || Size.y == 0)
            SP_CORE_ERROR("Renderer: ViewPort Size Given Not Valid!")
        _Renderer->_VulkanApi->SetViewPort(Size);
    }

    void Renderer::SetClearColor(const Vec4 &Color)
    {
        _Renderer->_VulkanApi->SetClearColor(Color);
    }

    void Renderer::SetFramesToRender(int n)
    {
        _Renderer->_VulkanApi->SetFramesToRender(n);
    }

    void Renderer::OnWindowResized()
    {
        _Renderer->_VulkanApi->OnWindowResized();
    }

    void Renderer::Submit(const VertexBuffer &VB)
    {
        _Renderer->_RenderData._VB = &VB;


    }

    void Renderer::Stop()
    {
        _Renderer->_VulkanApi->Stop();
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
