#pragma once

#include <cstdint>

namespace eigen
{

    enum class Format
    {
        Unspecified = 0,
        BC1,
        BC2,
        BC3,
        BC4,
        BC5,
        BC6,
        BC7,
        RGBA8       = 0x10,
        RGB10_A2,
        RGBA16,
        RGBA16f,
        RGBA32f,
        D24_S8      = 0x20,
        D32f,
        D32f_S8,
        Count
    };

    inline bool IsDepthFormat(Format format)
    {
        return format == Format::D24_S8 || format == Format::D32f || format == Format::D32f_S8;
    }

}
