#pragma once

#include "UUID.h"
#include "BufferLayout.h"

namespace Space
{
    class VertexBuffer
    {
    public:
        VertexBuffer();
        ~VertexBuffer();

        void SetBufferSpec(const BufferSpecStack &Stack) { _Stack = Stack; }

        const BufferSpecStack &GetBufferSpec() const { return _Stack; }
        const UUID &GetID() const { return _ID; }
        VkBuffer GetHandle() const { return _VBuffer; }
        int GetSize() const { return _Size; }

        void Init(Vertex *Vertices, int Size);
        void Destroy();

        void Bind() const;

        operator VkBuffer() const { return _VBuffer; }

    private:
        BufferSpecStack _Stack;
        UUID _ID;

        VkBuffer _VBuffer = VK_NULL_HANDLE;
        VkDeviceMemory _Memory = VK_NULL_HANDLE;

        int _Size = 0;
        Vertex *_Vertices = nullptr;
    };
} // namespace Space
