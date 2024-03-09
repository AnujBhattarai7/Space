#pragma once

namespace Space
{
    class Shader
    {
    public:
        static void ReadFile(std::vector<char>& _Code, const char* &_FilePath);
        static Shader *Create(const char *VertexFilePath, const char *ShaderFilePath);

        virtual ~Shader() {}

        virtual void Init(const char *VertexFilePath, const char *ShaderFilePath) = 0;
        virtual void ShutDown() = 0;

        virtual void Bind() const = 0;
        virtual void UnBind() const = 0;

    private:
    };
} // namespace Space
