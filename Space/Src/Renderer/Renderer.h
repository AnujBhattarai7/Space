#pragma once

#include "RenderApiOverView.h"
#include "Shaders.h"

namespace Space
{
    class Renderer
    {
    public:
        static Renderer &Get() { return *_Renderer; }

        static void Init(RenderApiType _RenderApiType);
        static void ShutDown();

        static RenderApiType GetApiType() { return _Renderer->_ApiType; }

    private:
        static Renderer *_Renderer;

        RenderApiOverView _Api;
        RenderApiType _ApiType;
    };
} // namespace Space
