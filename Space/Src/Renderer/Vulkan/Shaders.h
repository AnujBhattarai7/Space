#pragma once

namespace Space
{
    enum ShaderTypes
    {
        SHADER_TYPES_FRAGMENT,
        SHADER_TYPES_RENDER = SHADER_TYPES_FRAGMENT,
        SHADER_TYPES_VERTEX,
        SHADER_TYPES_SIZE
    };

    class Shader
    {
    public:
        static void ReadFile(std::vector<char> &_Code, const char *&_FilePath);

        Shader(const char *VertexFilePath, const char *FragFilePath)
        {
            Init(VertexFilePath, FragFilePath);
        }

        Shader() {}
        ~Shader();

        void Init(const char *VFilePath, const char *FFilePath);
        void Destroy();

        std::array<VkShaderModule, (int)SHADER_TYPES_SIZE> GetShaderModules() const;
        std::array<VkPipelineShaderStageCreateInfo, (int)SHADER_TYPES_SIZE> GetShaderModulesStageInfo() const;

    private:
        void CreateShaderModule(VkShaderModule Shader, const std::vector<char> &Code);

        VkShaderModule _Shaders[(int)SHADER_TYPES_SIZE] = {VK_NULL_HANDLE};
    };
} // namespace Space
