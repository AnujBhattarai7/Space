#pragma once

#include "Shaders.h"
#include "VulkanDevices.h"

namespace Space
{
    enum ShaderTypes
    {
        FRAGMENT,
        VERTEX,
        SIZE
    };

    class VulkanShader : public Shader
    {
    public:

        VulkanShader(const char *VertexFilePath, const char *FragFilePath)
        {
            Init(VertexFilePath, FragFilePath);
        }
        ~VulkanShader() { ShutDown(); }

        virtual void Init(const char *VertexFilePath, const char *FragFilePath);
        virtual void ShutDown();

        virtual void Bind() const;
        virtual void UnBind() const;

        void _CompileShader(const Device& _Device, const std::vector<char>& _Code, int i);

    private:
        VkShaderModule _ShaderModules[SIZE];
        VkPipelineShaderStageCreateInfo _ShaderInfo[SIZE];
    };
}
