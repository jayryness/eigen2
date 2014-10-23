#pragma once

#include "../Renderer.h"
#include "DisplayDx11.h"
#include "commonDx11.h"
#include "core/Pool.h"
#include <type_traits>

namespace eigen
{

    struct Renderer::PlatformConfig
    {
        IDXGIAdapter*               adapter = nullptr;
    };

    struct Renderer::PlatformDetails
    {
        ComPtr<IDXGIAdapter>        adapter;
        ComPtr<IDXGIFactory>        dxgiFactory;
        ComPtr<ID3D11Device>        device;
        ComPtr<ID3D11DeviceContext> immContext;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    inline Renderer::PlatformDetails& Renderer::getPlatformDetails()
    {
        return (PlatformDetails&)_platformDetails;
    }

}
