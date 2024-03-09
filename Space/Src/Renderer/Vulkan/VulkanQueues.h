#pragma once

namespace Space
{
    enum class QueueFamilies
    {
        // Index which Stores the QueueFamilyIndex to the Graphics Command
        GRAPHIC,
        // Index which Stores the QueueFamilyIndex to the Render Or Present Command
        RENDER, // OR Present as called in Tutorials
        SIZE,
    };

    // Stores the index where the Given Family Commands are located
    struct QueueFamilyIndices
    {
        uint32_t Families[(int)QueueFamilies::SIZE];

        QueueFamilyIndices()
        {
            for (int i = 0; i < (int)QueueFamilies::SIZE; i++)
                Families[i] = 10000;
        }

        bool _Exist()
        {
            for (int i = 0; i < (int)QueueFamilies::SIZE; i++)
            {
                if(Families[i] == 10000)
                    return false;
            }
            
            return true;
        }
    };

    //  Queue Family Finding Funcs
    QueueFamilyIndices _FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR _Surface, const std::initializer_list<QueueFamilies> &_Families);
}