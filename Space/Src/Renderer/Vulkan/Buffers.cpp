#include "PCH.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "VertexBuffers.h"
#include "VulkanApi.h"

namespace Space
{
    VertexBuffer::VertexBuffer()
    {
    }

    VertexBuffer::~VertexBuffer()
    {
        if(_VBuffer != VK_NULL_HANDLE)
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
        _Size = Size;

        VkBufferCreateInfo BufferInfo{};
        BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // For Vertex Buffer
        BufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        // Size in bytes
        BufferInfo.size = Size * sizeof(Vertex);
        // Since we are only using VBO on Graphics queue a single queue
        BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(GetActiveDevice(), &BufferInfo, nullptr, &_VBuffer) != VK_SUCCESS)
            SP_CORE_ERROR("Vertex Buffer Creation Failed!!")

        // Send data to it

        VkMemoryRequirements MemoryReq{};
        vkGetBufferMemoryRequirements(GetActiveDevice(), _VBuffer, &MemoryReq);

        VkMemoryAllocateInfo AllocateInfo{};
        AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        AllocateInfo.memoryTypeIndex = FindMemoryType(MemoryReq.memoryTypeBits,
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        AllocateInfo.allocationSize = MemoryReq.size;

        if (vkAllocateMemory(GetActiveDevice(), &AllocateInfo, nullptr, &_Memory) != VK_SUCCESS)
            SP_CORE_ERROR("Vertex Buffer Memory On Gpu not allocated!!")

        vkBindBufferMemory(GetActiveDevice(), _VBuffer, _Memory, 0);

        void *data;
        vkMapMemory(GetActiveDevice(), _Memory, 0, BufferInfo.size, 0, &data);

        memcpy(data, Vertices, (size_t)BufferInfo.size);
        vkUnmapMemory(GetActiveDevice(), _Memory);
    }

    void VertexBuffer::Destroy()
    {
        vkDestroyBuffer(GetActiveDevice(), _VBuffer, nullptr);
        vkFreeMemory(GetActiveDevice(), _Memory, nullptr);
        
        _VBuffer = VK_NULL_HANDLE;
    }
} // namespace Space
