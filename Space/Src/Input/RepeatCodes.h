#pragma once

// Contains the GLFW Input Codes which are repeated

// ---- Actions ----
#define SP_RELEASE 0
#define SP_PRESS 1
#define SP_REPEAT 2

namespace Space
{
    // Mouse Button Codes to String for debugging
    static const char *GetActionName(int button)
    {
        switch (button)
        {
        case SP_RELEASE:
            return "RELEASE";
        case SP_REPEAT:
            return "REPEAT";
        case SP_PRESS:
            return "PRESS";
        default:
            return "";
        };
    }

    enum class KeyMods
    {
        None = 0,
        SHIFT,
        ALT,
        CAPS_LOCK,
        CONTROL,
        NUM_LOCK,
        SUPER
    };
}
