#include "PCH.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "VulkanApi.h"
#include "VulkanShader.h"

namespace Space
{
    // Only for Now
    void Shader::ReadFile(std::vector<char> &_Code, const char *&_FilePath)
    {
        std::ifstream file(_FilePath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            SP_CORE_ERROR("Shader: ReadFile: File Not Found!!")

        size_t fileSize = (size_t)file.tellg();
        _Code.resize(fileSize);

        file.seekg(0);
        file.read(_Code.data(), fileSize);

        file.close();
    }

    void VulkanShader::Init(const char *VertexFilePath, const char *FragFilePath)
    {
        std::vector<char> _VertexSrc, _FragSrc;
        ReadFile(_VertexSrc, VertexFilePath);
        ReadFile(_FragSrc, FragFilePath);

        _CompileShader(_GetActiveDevice(), _VertexSrc, VERTEX);
        _CompileShader(_GetActiveDevice(), _FragSrc, FRAGMENT);

        _ShaderInfo[VERTEX].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        _ShaderInfo[VERTEX].stage = VK_SHADER_STAGE_VERTEX_BIT;
        _ShaderInfo[VERTEX].module = _ShaderModules[VERTEX];
        _ShaderInfo[VERTEX].pName = "main";

        _ShaderInfo[FRAGMENT].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        _ShaderInfo[FRAGMENT].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        _ShaderInfo[FRAGMENT].module = _ShaderModules[FRAGMENT];
        _ShaderInfo[FRAGMENT].pName = "main";
    }

    void VulkanShader::ShutDown()
    {
        vkDestroyShaderModule(_GetActiveDevice(), _ShaderModules[VERTEX], nullptr);
        vkDestroyShaderModule(_GetActiveDevice(), _ShaderModules[FRAGMENT], nullptr);
    }

    void VulkanShader::Bind() const
    {
    }

    void VulkanShader::UnBind() const
    {
    }

    void VulkanShader::_CompileShader(const Device &_Device, const std::vector<char> &_Code, int i)
    {
        VkShaderModuleCreateInfo _CreateInfo{};

        _CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        _CreateInfo.codeSize = _Code.size();
        _CreateInfo.pCode = reinterpret_cast<const uint32_t *>(_Code.data());

        if (vkCreateShaderModule(_Device.GetHandle(), &_CreateInfo, nullptr, &_ShaderModules[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create shader module!");

        if (i == VERTEX)
            SP_CORE_PRINT("Shader: Vertex Created")
        if (i == FRAGMENT)
            SP_CORE_PRINT("Shader: Fragment Created")
    }
}
