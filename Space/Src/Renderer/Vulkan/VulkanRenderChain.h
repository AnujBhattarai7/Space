#pragma once

namespace Space
{
    struct RenderChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanRenderChain
    {
    public:
        VulkanRenderChain() {}
        VulkanRenderChain(VkSurfaceKHR _Surface, VulkanDevice &_Device) { Init(_Surface, _Device); }
        ~VulkanRenderChain();

        void Init(VkSurfaceKHR _Surface, VulkanDevice &_Device);
        void Destroy(VulkanDevice &_Device);

        void _InitImageViews(VulkanDevice &_Device);

        operator VkSwapchainKHR() const { return _SwapChain; }

        const std::vector<VkImageView> &GetImageViews() const;
        VkSwapchainKHR GetHandle() const { return _SwapChain; }

        VkFormat GetFormat() const { return _Format; }
        VkExtent2D GetExtent() const { return _Extent; }
        uint32_t GetImageCount() const { return _ImageCount; }

    private:
        VkSwapchainKHR _SwapChain = VK_NULL_HANDLE;
        uint32_t _ImageCount = 0;

        VkFormat _Format;
        VkExtent2D _Extent;
    };

    RenderChainSupportDetails _GetSwapChainSupportDetails(VkSurfaceKHR _Surface, VkPhysicalDevice device);
    VkPresentModeKHR _ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkSurfaceFormatKHR _ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkExtent2D _ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

} // namespace Space
