#include "PCH.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "VulkanApi.h"

#include "Shaders.h"

namespace Space
{
    void Shader::ReadFile(std::vector<char> &_Code, const char *&_FilePath)
    {
        std::ifstream file(_FilePath, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            SP_CORE_ERROR("failed to open file!");
        }

        size_t fileSHADER_TYPES_SIZE = (size_t)file.tellg();
        _Code.reserve(fileSHADER_TYPES_SIZE);

        file.seekg(0);
        file.read(_Code.data(), fileSHADER_TYPES_SIZE);

        file.close();
    }

    Shader::~Shader()
    {
        Destroy();
    }

    void Shader::Init(const char *VFilePath, const char *FFilePath)
    {
        std::vector<char> VertCode, FragCode;
        ReadFile(VertCode, VFilePath);
        ReadFile(FragCode, FFilePath);

        CreateShaderModule(_Shaders[SHADER_TYPES_VERTEX], VertCode);
        CreateShaderModule(_Shaders[SHADER_TYPES_FRAGMENT], FragCode);
    }

    void Shader::Destroy()
    {
        if(_Shaders[SHADER_TYPES_VERTEX] == VK_NULL_HANDLE || _Shaders[SHADER_TYPES_FRAGMENT] == VK_NULL_HANDLE)
            return;
        
        vkDestroyShaderModule(GetActiveDevice(), _Shaders[SHADER_TYPES_VERTEX], nullptr);
        vkDestroyShaderModule(GetActiveDevice(), _Shaders[SHADER_TYPES_FRAGMENT], nullptr);
        _Shaders[SHADER_TYPES_FRAGMENT] = VK_NULL_HANDLE;
        _Shaders[SHADER_TYPES_VERTEX] = VK_NULL_HANDLE;
    }

    std::array<VkShaderModule, (int)ShaderTypes::SHADER_TYPES_SIZE> Shader::GetShaderModules() const
    {
        std::array<VkShaderModule, (int)ShaderTypes::SHADER_TYPES_SIZE> Ret;
        Ret[SHADER_TYPES_VERTEX] = _Shaders[SHADER_TYPES_VERTEX];
        Ret[SHADER_TYPES_FRAGMENT] = _Shaders[SHADER_TYPES_FRAGMENT];
        return Ret;
    }

    std::array<VkPipelineShaderStageCreateInfo, (int)ShaderTypes::SHADER_TYPES_SIZE> Shader::GetShaderModulesStageInfo() const
    {
        std::array<VkPipelineShaderStageCreateInfo, (int)ShaderTypes::SHADER_TYPES_SIZE> ShaderStages;

        ShaderStages[SHADER_TYPES_VERTEX].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStages[SHADER_TYPES_VERTEX].stage = VK_SHADER_STAGE_VERTEX_BIT;
        ShaderStages[SHADER_TYPES_VERTEX].module = _Shaders[SHADER_TYPES_VERTEX];
        ShaderStages[SHADER_TYPES_VERTEX].pName = "main";

        ShaderStages[SHADER_TYPES_FRAGMENT].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStages[SHADER_TYPES_FRAGMENT].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        ShaderStages[SHADER_TYPES_FRAGMENT].module = _Shaders[SHADER_TYPES_FRAGMENT];
        ShaderStages[SHADER_TYPES_FRAGMENT].pName = "main";

        return ShaderStages;
    }

    void Shader::CreateShaderModule(VkShaderModule Shader, const std::vector<char> &Code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = Code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(Code.data());

        if (vkCreateShaderModule(GetActiveDevice(), &createInfo, nullptr, &Shader) != VK_SUCCESS)
            throw std::runtime_error("failed to create shader module!");
    }
}
