#pragma once

#include "Core.h"

namespace Space
{
    class RenderApi
    {
    public:
        virtual ~RenderApi() {}

        virtual bool Init() = 0;
        virtual bool ShutDown() = 0;

        RenderApiType GetApiType() { return _ApiType; }

    protected:
        RenderApiType _ApiType;
    };
} // namespace Space
