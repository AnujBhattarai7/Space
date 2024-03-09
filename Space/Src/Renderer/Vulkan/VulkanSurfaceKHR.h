#pragma once

namespace Space
{
    // A Class to store the Surface Details such as The Surface Handle
    class SurfaceKHR
    {
    public:
        SurfaceKHR() {}
        ~SurfaceKHR();

        // VkInstance Required as so that other classes will not be able to destroy the
        // Surface

        void Init(VkInstance _VKInstance);
        void Destroy(VkInstance _VKInstance);

        VkSurfaceKHR GetHandle() { return _Surface; }

        operator VkSurfaceKHR() const { return _Surface; }

    private:
        VkSurfaceKHR _Surface = VK_NULL_HANDLE;
    };
}
