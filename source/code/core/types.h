#pragma once

#include <cstdint>

namespace eigen
{
    template<typename T, int N> struct FixedN
    {
        typedef T       Type;
        enum {          Dimension = N };

        static FixedN   Xyzw(T x, T y, T z, T w);
        static FixedN   Xyz(T x, T y, T z);
        static FixedN   Xy(T x, T y);

        T              _[N];
    };

    typedef FixedN<float, 2>    Float2;
    typedef FixedN<float, 3>    Float3;
    typedef FixedN<float, 4>    Float4;

    typedef FixedN<int32_t, 2>  Int2;
    typedef FixedN<int32_t, 3>  Int3;
    typedef FixedN<int32_t, 4>  Int4;

    template<typename T, int N> FixedN<T,N> FixedN<T,N>::Xyzw(T x, T y, T z, T w)
    {
        FixedN<T,N> result;
        result._[0] = x;
        result._[1] = y;
        result._[2] = z;
        result._[3] = w;
        return result;
    }

    template<typename T, int N> FixedN<T,N> FixedN<T,N>::Xyz(T x, T y, T z)
    {
        FixedN<T,N> result;
        result._[0] = x;
        result._[1] = y;
        result._[2] = z;
        return result;
    }

    template<typename T, int N> FixedN<T,N> FixedN<T,N>::Xy(T x, T y)
    {
        FixedN<T,N> result;
        result._[0] = x;
        result._[1] = y;
        return result;
    }
}
