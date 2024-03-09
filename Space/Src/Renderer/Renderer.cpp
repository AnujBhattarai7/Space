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
        if(Renderer::GetApiType() == RenderApiType::VULKAN_1)
            return new VulkanShader(VertexFilePath, ShaderFilePath);
        if(Renderer::GetApiType() == RenderApiType::VULKAN_2)
            return new VulkanShader(VertexFilePath, ShaderFilePath);
        
        SP_CORE_ERROR("Shader: Given Api Not Supoorted")
        return nullptr;
    }
}
