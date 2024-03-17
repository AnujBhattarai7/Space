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
        // Just allocate memory for the Buffer
        void Init(int Size);
        void Destroy();
        
        // Streams the Data to the GPU
        void Stream(Vertex* Vertices, int Size);

        operator VkBuffer() const { return _VBuffer; }

    private:
        BufferSpecStack _Stack;
        UUID _ID;

        VkBuffer _VBuffer = VK_NULL_HANDLE;
        VkDeviceMemory _Memory = VK_NULL_HANDLE;

        int _Size = 0, _SizeInBytes;
        Vertex *_Vertices = nullptr;
    };

    class IndexBuffer
    {
    public:
        IndexBuffer();
        ~IndexBuffer();

        const UUID &GetID() const { return _ID; }
        VkBuffer GetHandle() const { return _IBuffer; }
        int GetSize() const { return _Size; }

        void Init(uint16_t  *Indices, int Size);
        // Just allocate memory for the Buffer
        void Init(int Size);
        void Destroy();

        // Streams the Data to the GPU
        void Stream(uint16_t *Indices, int Size);

        operator VkBuffer() const { return _IBuffer; }

    private:
        BufferSpecStack _Stack;
        UUID _ID;

        VkBuffer _IBuffer = VK_NULL_HANDLE;
        VkDeviceMemory _Memory = VK_NULL_HANDLE;

        int _Size = 0, _SizeInBytes;
        uint16_t  *_Indices = nullptr;
    };
} // namespace Space
