#pragma once

namespace Space
{
    const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

#if (SP_DEBUG)
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

    enum VulkanApiVersion
    {
        VULKAN_1_0,
        VULKAN_1_1,
        VULKAN_1_2,
        VULKAN_1_3,
        VULKAN_2_0
    };

} // namespace Space
