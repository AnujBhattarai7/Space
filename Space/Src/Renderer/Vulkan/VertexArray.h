#pragma once

#include <stdint.h>

#include "Maths.h"
#include "Buffers.h"

namespace Space
{
    class VertexArray
    {
    public:
        VertexArray() {}
        ~VertexArray() { Destroy(); }

        void Destroy()
        {
            _VBO.Destroy();
            _IBO.Destroy();
        }

        void AddVB(int Size) { _VBO.Init(Size); }
        void AddVB(Vertex *Vertices, int Size) { _VBO.Init(Vertices, Size); }

        void AddIB(int Size) { _IBO.Init(Size); }
        void AddIB(uint16_t *Indices, int Size) { _IBO.Init(Indices, Size); }

        VertexBuffer &GetVB() { return _VBO; }
        const VertexBuffer &GetVB() const { return _VBO; }

        IndexBuffer &GetIB() { return _IBO; }
        const IndexBuffer &GetIB() const { return _IBO; }

    private:
        VertexBuffer _VBO;
        IndexBuffer _IBO;
    };
} // namespace Space
