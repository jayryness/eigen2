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

        _displayManager.initialize<DisplayDx11>(config.allocator, 8);
        _textureManager.initialize<TextureDx11>(config.allocator, 128);
        _targetSetManager.initialize<TargetSetDx11>(config.allocator, 16);

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
        DisplayDx11* display = _displayManager.create<DisplayDx11>();
        return display;
    }

    TexturePtr Renderer::createTexture()
    {
        TextureDx11* texture = _textureManager.create<TextureDx11>();
        return texture;
    }

    TargetSetPtr Renderer::createTargetSet()
    {
        TargetSetDx11* targetSet = _targetSetManager.create<TargetSetDx11>();
        return targetSet;
    }

    void DestroyRefCounted(Display* display)
    {
        display->getManager()->discard(display);
    }

    void DestroyRefCounted(Texture* texture)
    {
        texture->getManager()->discard(texture);
    }

    void DestroyRefCounted(TargetSet* targetSet)
    {
        targetSet->getManager()->discard(targetSet);
    }


}
