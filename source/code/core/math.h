#pragma once

#include <stdint.h>

namespace eigen
{
    unsigned LocateBit(uint64_t n);
    unsigned LocateBit(uint32_t n);

    uint64_t FloodBitsRight(uint64_t n);
    uint32_t FloodBitsRight(uint32_t n);

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

    inline unsigned LocateBit(uint64_t n)
    {
        unsigned pos = 0;
        pos += ((n & 0x00000000ffffffff) == 0) * 32;
        pos += ((n & 0x0000ffff0000ffff) == 0) * 16;
        pos += ((n & 0x00ff00ff00ff00ff) == 0) * 8;
        pos += ((n & 0x0f0f0f0f0f0f0f0f) == 0) * 4;
        pos += ((n & 0x3333333333333333) == 0) * 2;
        pos += ((n & 0x5555555555555555) == 0) * 1;
        return pos;
    }

    inline unsigned LocateBit(uint32_t n)
    {
        unsigned pos = 0;
        pos += ((n & 0x0000ffff) == 0) * 16;
        pos += ((n & 0x00ff00ff) == 0) * 8;
        pos += ((n & 0x0f0f0f0f) == 0) * 4;
        pos += ((n & 0x33333333) == 0) * 2;
        pos += ((n & 0x55555555) == 0) * 1;
        return pos;
    }

    inline uint64_t FloodBitsRight(uint64_t n)
    {
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n;
    }

    inline uint32_t FloodBitsRight(uint32_t n)
    {
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n;
    }

}
