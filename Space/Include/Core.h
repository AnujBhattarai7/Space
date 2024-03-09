#pragma once

// Only for debug print
#if (SP_DEBUG)
#define SP_BUILD_LOGS
#endif

namespace Space
{
    enum class RenderApiType
    {
        VULKAN_1,
        VULKAN_2,
        OPENGL_2,
        OPENGL_3
    };

    enum class BUILD_TYPE
    {
        DEBUG,
        RELEASE,
        DIST
    };

    RenderApiType GetActiveRenderApi();

    inline static BUILD_TYPE GetBuildType()
    {
#if (SP_DEBUG)
        return BUILD_TYPE::DEBUG;
#else
        return BUILD_TYPE::RELEASE;
#endif
    }
} // namespace Space
