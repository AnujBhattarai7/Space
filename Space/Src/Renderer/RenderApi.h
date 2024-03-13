#pragma once

#include "Core.h"
#include "Maths.h"

namespace Space
{
    class RenderApi
    {
    public:
        virtual ~RenderApi() {}

        virtual bool Init() = 0;
        virtual bool ShutDown() = 0;
        virtual void Update() = 0;

        RenderApiType GetApiType() { return _ApiType; }

        virtual void Begin(const Vec2& _VWSize) = 0;
        virtual void End() = 0;

        virtual void Render() = 0;

    protected:
        RenderApiType _ApiType;
    };
} // namespace Space
