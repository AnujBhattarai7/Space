// The main Header File which stores the Classes used for Rendering such as The ImageView,
// Image, SwapChain

#pragma once

#include "VulkanSurfaceKHR.h"

namespace Space
{
    struct RenderChainData
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        bool _IsSuitable()
        {
            return (formats.size() > 0) && (presentModes.size() > 0);
        }
    };

    static std::vector<VkImage> _RenderImages;
    static std::vector<VkImageView> _RenderImageViews;
    // The SwapChain Which stores all of the Images which are rendered
    class RenderChain
    {
    public:
        RenderChain() {}
        ~RenderChain();

        void Init(VkPhysicalDevice _PDevice, VkSurfaceKHR _Surface, VkDevice _Device);
        void Destroy(VkDevice _Device);

        void _InitSwapChainTarget(VkPhysicalDevice _PDevice, VkSurfaceKHR _Surface, VkDevice _Device);
        void _InitSwapChainImages(VkDevice _Device);

    private:
        UUID _ID, _DeviceID;

        VkSwapchainKHR _SwapChain = VK_NULL_HANDLE;
        // The Result Images of the SwapChain
        VkFormat _SwapChainImageFormat;
        VkExtent2D _SwapChainExtent;

        // std::vector<VkImage> _RenderImages;
        // std::vector<VkImageView> _RenderImageViews;
    };

    // Swap Chain
    RenderChainData _GetRenderchainSupport(VkPhysicalDevice _Device, VkSurfaceKHR _Surface);
    VkSurfaceFormatKHR _GetSuitableRenderSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &_Formats);
    VkPresentModeKHR _GetSuitableRenderSurfacePresentMode(const std::vector<VkPresentModeKHR> &_PresentModess);
    VkExtent2D _GetSuitableRenderExtent(const VkSurfaceCapabilitiesKHR &_Capabilites);
} // namespace Space
