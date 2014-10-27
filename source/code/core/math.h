#pragma once

namespace eigen
{
    template<unsigned N> struct StaticLog2;
    template<unsigned N> struct StaticNextPow2;

    template<unsigned N> struct StaticLog2
    {
        enum { Result = 1 + StaticLog2<N / 2>::Result };
    };

    template<> struct StaticLog2<1>
    {
        enum { Result = 0 };
    };

    template<unsigned N> struct StaticNextPow2
    {
        enum { Result = 2 * StaticNextPow2<(N + 1) / 2>::Result };
    };

    template<> struct StaticNextPow2<1>
    {
        enum { Result = 1 };
    };
}
