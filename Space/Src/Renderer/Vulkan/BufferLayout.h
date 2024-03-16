#pragma once

#include "Maths.h"

namespace Space
{
    enum class Types
    {
        FLOAT,
        FLOAT64,
        UINT16,
        UINT32,
        UINT64,
        UINT128,
        INT,
        INT16,
        INT64,
        INT128
    };

    static const int GetSizeTypes(Types _T)
    {
        switch (_T)
        {
        case Space::Types::FLOAT:
            return sizeof(float);
        case Space::Types::FLOAT64:
            return sizeof(double);
        case Space::Types::UINT16:
            return sizeof(uint16_t);
        case Space::Types::UINT32:
            return sizeof(uint32_t);
        case Space::Types::UINT64:
            return sizeof(uint64_t);
        case Space::Types::UINT128:
            return 16;
        case Space::Types::INT:
            return sizeof(int);
        case Space::Types::INT16:
            return sizeof(int16_t);
        case Space::Types::INT64:
            return sizeof(int64_t);
        case Space::Types::INT128:
            return 16;
        default:
            break;
        }
    }

    // The Buffer Specification for a stride
    struct BufferSpecifiaction
    {
    public:
        BufferSpecifiaction(Types T, int Dim) { Init(T, Dim); }
        BufferSpecifiaction() {}

        ~BufferSpecifiaction() {}

        inline int GetDim() const { return _D; }
        inline int GetStride() const { return _S; }
        inline int GetNomralized() const { return _N; }
        inline Types GetType() const { return _T; }

        inline void Init(Types T, int Dim)
        {
            if (T == Types::FLOAT || T == Types::FLOAT64)
                _N = true;
            _T = T;
            _D = Dim;
            _S = GetSizeTypes(T) * Dim;
        }

    private:
        int _S, _D;
        Types _T;
        bool _N = false;
    };

    // Stores the Buffer Specifications
    struct BufferSpecStack
    {
    public:
        BufferSpecStack(const std::initializer_list<BufferSpecifiaction> &Specs) { AddSpecification(Specs); }
        BufferSpecStack() {}
        ~BufferSpecStack() {}

        inline const BufferSpecifiaction &GetSpecification(int i) const { return _Specs[i]; }
        inline int Size() const { return _Specs.size(); }
        inline int GetTotalStride() const
        {
            int _Stride = 0;
            for (auto &i : _Specs)
                _Stride += i.GetStride();

            return _Stride;
        }

        inline void AddSpecification(const std::initializer_list<BufferSpecifiaction> &Specs)
        {
            for (auto &i : Specs)
                _Specs.push_back(i);
        }

    private:
        std::vector<BufferSpecifiaction> _Specs;
    };

} // namespace Space