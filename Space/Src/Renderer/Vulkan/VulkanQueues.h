#pragma once

namespace Space
{
    enum QueueFamilyIndex
    {
        PRESENT,
        RENDER = 0,
        GRAPHICS,
        COMPUTE,
        TRANSFER,
        SIZE
    };

    struct QueueFamilyIndices
    {
        uint32_t _Indicies[QueueFamilyIndex::SIZE];

        QueueFamilyIndices()
        {
            for (auto &i : _Indicies)
                i = 900000000000;
        }

        bool IsInSameFamily()
        {
            for (int i = 0; i < SIZE - 1; i++)
                if (_Indicies[i] != _Indicies[i + 1])
                    return false;

            return true;
        }

        bool isComplete()
        {
            for (auto &i : _Indicies)
                if (i == 900000000000)
                    return false;
            return true;
        }
    };

    QueueFamilyIndices _GetDeviceQueueFamilies(VkPhysicalDevice _Device, VkSurfaceKHR _Surface);

} // namespace Space
