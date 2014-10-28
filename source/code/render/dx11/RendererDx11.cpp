#include "RendererDx11.h"
#include "DisplayDx11.h"
#include "TargetSetDx11.h"
#include <cassert>
#include <new>

namespace eigen
{
    Error Renderer::platformInit(const Config& config)
    {
        static_assert(sizeof(PlatformDetails) <= sizeof(Renderer::_platformDetails), "Must increase size of Renderer::_platformDetails");

        _displayAllocator.initialize(config.allocator, sizeof(DisplayDx11), 8);
        _textureAllocator.initialize(config.allocator, sizeof(TextureDx11), 64);
        _targetSetAllocator.initialize(config.allocator, sizeof(TargetSetDx11), 16);

        PlatformDetails& plat = getPlatformDetails();
        new (&plat) PlatformDetails();
        if (config.platformConfig)
        {
            plat.adapter = config.platformConfig->adapter;
        }

        D3D_FEATURE_LEVEL featureLevels [] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

        HRESULT hr = D3D11CreateDevice(
            plat.adapter.Get(),
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            _config.debugEnabled ? D3D11_CREATE_DEVICE_DEBUG : 0,
            featureLevels,
            sizeof(featureLevels) / sizeof(*featureLevels),
            D3D11_SDK_VERSION,
            &plat.device,
            NULL,
            &plat.immContext);

        if (FAILED(hr))
        {
            EIGEN_RETURN_ERROR("Failed to create D3D device, HRESULT = %d", hr);
        }

        // Retrieve dxgi factory
        {
            plat.adapter.Detach();
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = plat.device.Get()->QueryInterface(__uuidof(dxgiDevice), &dxgiDevice);
            hr = dxgiDevice.Get()->GetParent(__uuidof(IDXGIAdapter), &plat.adapter);
            hr = plat.adapter.Get()->GetParent(__uuidof(IDXGIFactory), &plat.dxgiFactory);
        }

        if (FAILED(hr))
        {
            EIGEN_RETURN_ERROR("Failed to retrieve DXGI factory, HRESULT = %d", hr);
        }

        EIGEN_RETURN_OK();
    }

    void Renderer::platformCleanup()
    {
        PlatformDetails& plat = getPlatformDetails();
        plat.~PlatformDetails();
    }

    DisplayPtr Renderer::createDisplay()
    {
        DisplayDx11* display = new(AllocateMemory<DisplayDx11>(&_displayAllocator, 1)) DisplayDx11(*this);
        return display;
    }

    TexturePtr Renderer::createTexture()
    {
        TextureDx11* texture = new(AllocateMemory<TextureDx11>(&_textureAllocator, 1)) TextureDx11(*this);
        return texture;
    }

    TargetSetPtr Renderer::createTargetSet()
    {
        TargetSetDx11* targetSet = new(AllocateMemory<TargetSetDx11>(&_targetSetAllocator, 1)) TargetSetDx11(*this);
        return targetSet;
    }

    void DestroyRefCounted(Display* display)
    {
        Renderer& renderer = ((DisplayDx11*)display)->_renderer;
        renderer.scheduleDeletion((DisplayDx11*)display, 1);
    }

    void DestroyRefCounted(Texture* texture)
    {
        Renderer& renderer = ((TextureDx11*)texture)->_renderer;
        renderer.scheduleDeletion((TextureDx11*)texture, 1);
    }

    void DestroyRefCounted(TargetSet* targetSet)
    {
        Renderer& renderer = ((TargetSetDx11*)targetSet)->_renderer;
        renderer.scheduleDeletion((TargetSetDx11*)targetSet, 1);
    }


}
