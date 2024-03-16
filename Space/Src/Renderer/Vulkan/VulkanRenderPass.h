#pragma once

#include "UUID.h"

namespace Space
{
    class VulkanRenderPass
    {
    public:
        VulkanRenderPass() {}
        VulkanRenderPass(VulkanDevice &_Device, VulkanRenderChain& _SwapChains) { Init(_Device, _SwapChains); }
        ~VulkanRenderPass();

        void Init(VulkanDevice& _Device, VulkanRenderChain& _SwapChains);
        void Destroy(VulkanDevice& _Device);

        operator VkRenderPass() const { return _RenderPass; }

        VkRenderPass GetHandle() const {return _RenderPass;}

    private:
        VkRenderPass _RenderPass = VK_NULL_HANDLE;
        UUID _ID;
    };
} // namespace Space
