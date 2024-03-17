#include "PCH.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "Buffers.h"
#include "VulkanApi.h"

namespace Space
{
    VertexBuffer::VertexBuffer()
    {
    }

    VertexBuffer::~VertexBuffer()
    {
        if (_VBuffer != VK_NULL_HANDLE)
            Destroy();
    }

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(GetActiveDevice().GetPhysicalDevice().GetHandle(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        SP_CORE_ERROR("Vertex Buffer Suitable Memory Type not Found!!")
    }

    void VertexBuffer::Init(Vertex *Vertices, int Size)
    {
        Init(Size);
        Stream(Vertices, Size);
    }

    void CreateBuffer(VkBuffer &Buffer, VkDeviceMemory &Memory, VkBufferUsageFlags Usage, int SizeInBytes, VkMemoryPropertyFlags properties)
    {
        VkBufferCreateInfo BufferInfo{};
        BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // For Vertex Buffer
        BufferInfo.usage = Usage;
        // Size in bytes
        BufferInfo.size = SizeInBytes;
        // Since we are only using VBO on Graphics queue a single queue
        BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(GetActiveDevice(), &BufferInfo, nullptr, &Buffer) != VK_SUCCESS)
            SP_CORE_ERROR("Vertex Buffer Creation Failed!!")

        // Send data to it
        VkMemoryRequirements MemoryReq{};
        vkGetBufferMemoryRequirements(GetActiveDevice(), Buffer, &MemoryReq);

        VkMemoryAllocateInfo AllocateInfo{};
        AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        AllocateInfo.memoryTypeIndex = FindMemoryType(MemoryReq.memoryTypeBits, properties);

        AllocateInfo.allocationSize = MemoryReq.size;

        if (vkAllocateMemory(GetActiveDevice(), &AllocateInfo, nullptr, &Memory) != VK_SUCCESS)
            SP_CORE_ERROR("Vertex Buffer Memory On Gpu not allocated!!")
    }

    void VertexBuffer::Init(int Size)
    {
        _Size = Size;
        _SizeInBytes = Size * sizeof(Vertex);

        CreateBuffer(_VBuffer, _Memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, Size * sizeof(Vertex),
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    }

    void VertexBuffer::Destroy()
    {
        vkDestroyBuffer(GetActiveDevice(), _VBuffer, nullptr);
        vkFreeMemory(GetActiveDevice(), _Memory, nullptr);

        _VBuffer = VK_NULL_HANDLE;
    }

    void VertexBuffer::Stream(Vertex *Vertices, int Size)
    {
        if (Size != _Size)
            SP_CORE_ERROR("VertexBuffer: Given Size is not equal to the Size given for Init!!")

        vkBindBufferMemory(GetActiveDevice(), _VBuffer, _Memory, 0);

        void *data;
        vkMapMemory(GetActiveDevice(), _Memory, 0, _SizeInBytes, 0, &data);

        memcpy(data, Vertices, (size_t)_SizeInBytes);
        vkUnmapMemory(GetActiveDevice(), _Memory);
    }

    IndexBuffer::IndexBuffer()
    {
    }

    IndexBuffer::~IndexBuffer()
    {
        if (_IBuffer != VK_NULL_HANDLE)
            Destroy();
    }

    void IndexBuffer::Init(uint16_t *Indices, int Size)
    {
        Init(Size);
        Stream(Indices, Size);
    }

    void IndexBuffer::Init(int Size)
    {
        _Size = Size;
        _SizeInBytes = Size * sizeof(uint16_t);

        CreateBuffer(_IBuffer, _Memory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, Size * sizeof(uint16_t),
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    }

    void IndexBuffer::Destroy()
    {
        vkDestroyBuffer(GetActiveDevice(), _IBuffer, nullptr);
        vkFreeMemory(GetActiveDevice(), _Memory, nullptr);

        _IBuffer = VK_NULL_HANDLE;
    }

    void IndexBuffer::Stream(uint16_t *Indices, int Size)
    {
        if (Size != _Size)
            SP_CORE_ERROR("VertexBuffer: Given Size is not equal to the Size given for Init!!")

        vkBindBufferMemory(GetActiveDevice(), _IBuffer, _Memory, 0);

        void *data;
        vkMapMemory(GetActiveDevice(), _Memory, 0, _SizeInBytes, 0, &data);

        memcpy(data, Indices, (size_t)_SizeInBytes);
        vkUnmapMemory(GetActiveDevice(), _Memory);
    }
} // namespace Space
