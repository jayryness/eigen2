#include "../Format.h"
#include "commonDx11.h"

namespace eigen
{

    inline DXGI_FORMAT TranslateFormat(Format format)
    {
        switch (format)
        {
        case Format::BC1:       return DXGI_FORMAT_BC1_TYPELESS;
        case Format::BC2:       return DXGI_FORMAT_BC2_TYPELESS;
        case Format::BC3:       return DXGI_FORMAT_BC3_TYPELESS;
        case Format::BC4:       return DXGI_FORMAT_BC4_TYPELESS;
        case Format::BC5:       return DXGI_FORMAT_BC5_TYPELESS;
        case Format::BC6:       return DXGI_FORMAT_BC6H_TYPELESS;
        case Format::BC7:       return DXGI_FORMAT_BC7_TYPELESS;
        case Format::RGBA8:     return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case Format::RGB10_A2:  return DXGI_FORMAT_R10G10B10A2_TYPELESS;
        case Format::RGBA16:    return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case Format::RGBA16f:   return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case Format::RGBA32f:   return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case Format::D24_S8:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case Format::D32f:      return DXGI_FORMAT_D32_FLOAT;
        case Format::D32f_S8:   return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:                return DXGI_FORMAT_UNKNOWN;
        }
    }

    inline Format TranslateFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_BC1_TYPELESS:          return Format::BC1;
        case DXGI_FORMAT_BC1_UNORM:             return Format::BC1;
        case DXGI_FORMAT_BC1_UNORM_SRGB:        return Format::BC1;
        case DXGI_FORMAT_BC2_TYPELESS:          return Format::BC2;
        case DXGI_FORMAT_BC2_UNORM:             return Format::BC2;
        case DXGI_FORMAT_BC2_UNORM_SRGB:        return Format::BC2;
        case DXGI_FORMAT_BC3_TYPELESS:          return Format::BC3;
        case DXGI_FORMAT_BC3_UNORM:             return Format::BC3;
        case DXGI_FORMAT_BC3_UNORM_SRGB:        return Format::BC3;
        case DXGI_FORMAT_BC4_TYPELESS:          return Format::BC4;
        case DXGI_FORMAT_BC4_UNORM:             return Format::BC4;
        case DXGI_FORMAT_BC4_SNORM:             return Format::BC4;
        case DXGI_FORMAT_BC5_TYPELESS:          return Format::BC5;
        case DXGI_FORMAT_BC5_UNORM:             return Format::BC5;
        case DXGI_FORMAT_BC5_SNORM:             return Format::BC5;
        case DXGI_FORMAT_BC6H_TYPELESS:         return Format::BC6;
        case DXGI_FORMAT_BC6H_UF16:             return Format::BC6;
        case DXGI_FORMAT_BC6H_SF16:             return Format::BC6;
        case DXGI_FORMAT_BC7_TYPELESS:          return Format::BC7;
        case DXGI_FORMAT_BC7_UNORM:             return Format::BC7;
        case DXGI_FORMAT_BC7_UNORM_SRGB:        return Format::BC7;
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return Format::RGBA8;
        case DXGI_FORMAT_R8G8B8A8_UNORM:        return Format::RGBA8;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return Format::RGBA8;
        case DXGI_FORMAT_R8G8B8A8_UINT:         return Format::RGBA8;
        case DXGI_FORMAT_R8G8B8A8_SNORM:        return Format::RGBA8;
        case DXGI_FORMAT_R8G8B8A8_SINT:         return Format::RGBA8;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return Format::RGB10_A2;
        case DXGI_FORMAT_R10G10B10A2_UNORM:     return Format::RGB10_A2;
        case DXGI_FORMAT_R10G10B10A2_UINT:      return Format::RGB10_A2;
        case DXGI_FORMAT_R16G16B16A16_UNORM:    return Format::RGBA16;
        case DXGI_FORMAT_R16G16B16A16_UINT:     return Format::RGBA16;
        case DXGI_FORMAT_R16G16B16A16_SNORM:    return Format::RGBA16;
        case DXGI_FORMAT_R16G16B16A16_SINT:     return Format::RGBA16;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:    return Format::RGBA16f;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:    return Format::RGBA32f;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:     return Format::D24_S8;
        case DXGI_FORMAT_D32_FLOAT:             return Format::D32f;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:  return Format::D32f_S8;
        default:                                return Format::Unspecified;
        }
    }

}
