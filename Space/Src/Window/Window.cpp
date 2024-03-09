#include "PCH.h"
#include "Window.h"
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_core.h>

namespace Space
{
    int _SHUT_GLFW()
    {
        // Terminates GLFW operations
        if (_GLFW_INIT == false)
        {
            SP_CORE_PRINT("GLFW Already ShutDown!!");
            return true;
        }

        glfwTerminate();
        _GLFW_INIT = false;

        SP_CORE_PRINT("GLFW ShutDown!!")

        return true;
    }

    void SetRequiredExtensions(std::vector<const char *> &_Extensions)
    {
        if (GetActiveRenderApi() != RenderApiType::VULKAN_1 && GetActiveRenderApi() != RenderApiType::VULKAN_2)
            return;

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        _Extensions.reserve(glfwExtensionCount + 1);
        for (int i = 0; i < glfwExtensionCount; i++)
            _Extensions.push_back(glfwExtensions[i]);

#if (SP_DEBUG)
        // For Debugging
        _Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    }
} // namespace Space
