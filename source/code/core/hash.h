#pragma once

#include <cstdint>
#include <cstring>

namespace eigen
{

    uint32_t StringHash32(const char* p);
    uint32_t Hash32(const char* p, int length);

    ///////////////////////////////////////////////////////////////////////////////////////////

    inline uint32_t StringHash32(const char* p)
    {
        uint32_t result = Hash32(p, (int)strlen(p));
        return result;
    }

    inline uint32_t Hash32(const char* p, int length)
    {
        // This is murmurhash3.

        uint32_t result = 0x52071a6c;

        const uint8_t * data = (const uint8_t*)p;
        const int nblocks = length / 4;

        const uint32_t c1 = 0xcc9e2d51;
        const uint32_t c2 = 0x1b873593;

        // body

        const uint32_t * blocks = (const uint32_t *) (data + nblocks * 4);

        for (int i = -nblocks; i; i++)
        {
            uint32_t k1 = blocks[i];

            k1 *= c1;
            k1 = (k1 << 15) | (k1 >> (32 - 15)); //ROTL32(k1, 15);
            k1 *= c2;

            result ^= k1;
            result = (result << 13) | (result >> (32 - 13)); //ROTL32(result, 13);
            result = result * 5 + 0xe6546b64;
        }

        // tail

        const uint8_t * tail = (const uint8_t*) (data + nblocks * 4);

        uint32_t k1 = 0;

        switch (length & 3)
        {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
            k1 *= c1; k1 = (k1 << 15) | (k1 >> (32 - 15)); k1 *= c2; result ^= k1;
        };

        // finalization

        result ^= length;

        result ^= result >> 16;
        result *= 0x85ebca6b;
        result ^= result >> 13;
        result *= 0xc2b2ae35;
        result ^= result >> 16;

        return result;
    }
}
