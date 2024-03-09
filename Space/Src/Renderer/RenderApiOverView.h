#pragma once

#include "RenderApi.h"

namespace Space
{
    // A Layer between the RenderApi such as OpenGL, Vulkan, etc... and Renderer
    class RenderApiOverView
    {
    public:
        RenderApiOverView() {}
        ~RenderApiOverView() { ShutDown(); }

        // The Function used by Renderer to specify which RenderApi to use
        void Init(RenderApi *_RenderApi)
        {
            // If _Api already exists then Delete
            if (_Api != nullptr)
                ShutDown();
            _Api = _RenderApi;
        }

        void ShutDown()
        {
            // If _Api exists the delete
            if (_Api != nullptr)
                delete _Api;
            _Api = nullptr;
        }

    private:
        RenderApi *_Api = nullptr;
    };
} // namespace Space
