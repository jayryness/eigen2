#include "DisplayDx11.h"
#include "RendererDx11.h"
#include "TextureDx11.h"
#include "core/memory.h"
#include "core/Error.h"

namespace eigen
{
    Error Display::bindToWindow(void* windowHandle)
    {
        DisplayDx11* display = (DisplayDx11*)this;
        ComPtr<IDXGISwapChain> swapChain;

        //TextureTarget::Format format  = TextureTarget::cFormat_RGB10_A2;
        DXGI_FORMAT dxgiFormat          = DXGI_FORMAT_R10G10B10A2_UNORM;

        DXGI_SWAP_CHAIN_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.SampleDesc.Count = 1;
        desc.BufferDesc.Format = dxgiFormat;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.OutputWindow = (HWND)windowHandle;
        desc.Windowed = true;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        Renderer& renderer = ((DisplayDx11*)this)->_renderer;

        Renderer::PlatformDetails& plat = renderer.getPlatformDetails();

        HRESULT hr = plat.dxgiFactory.Get()->CreateSwapChain(plat.device.Get(), &desc, &swapChain);
        if (FAILED(hr))
        {
            EIGEN_RETURN_ERROR("Failed to created SwapChain, HRESULT = %d", hr);
        }

        ComPtr<ID3D11Texture2D> surface = nullptr;
        hr = swapChain->GetBuffer(0, __uuidof(surface), (void**) surface.GetAddressOf());
        assert(hr == S_OK);

        display->_hwnd = (HWND)windowHandle;
        display->_target = renderer.createTexture();
        ((TextureDx11*)display->_target.ptr)->initWithTexture2D(surface.Get());

        display->_swapChain.Swap(swapChain);

        EIGEN_RETURN_OK();
    }

    void Display::present()
    {
        DisplayDx11* display = (DisplayDx11*)this;
        IDXGISwapChain* swapChain = display->_swapChain.Get();

        HRESULT hr = swapChain->Present(0, 0);
        assert(hr == S_OK);

        // While we're at it... resize target if window changed

        //RECT rect;
        //GetClientRect(display->_hwnd, &rect);

        //int width = rect.right - rect.left;
        //int height = rect.bottom - rect.top;

        //const Texture::Config& textureInfo = display->_target.ptr->getConfig();
        //if (textureInfo.width != width || textureInfo.height != height)
        //{
        //    display->_target.ptr->detach();
        //    swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        //    ComPtr<ID3D11Texture2D> surface = nullptr;
        //    hr = swapChain->GetBuffer(0, __uuidof(surface), (void**)surface.GetAddressOf());
        //    assert(hr == S_OK);

        // TODO TargetSets' RenderTargetViews still holding refs to it

        //    ((TextureDx11*)display->_target.ptr)->initWithTexture2D(surface.Get());
        //}
    }

    void DisplayManager::platformInit(Allocator* allocator)
    {
        _blockAllocator.initialize(allocator, sizeof(DisplayDx11), 8);
    }

    Display* DisplayManager::createDisplay()
    {
        Renderer* renderer = StructFromMember(&Renderer::_displayManager, this);
        DisplayDx11* display = new(AllocateMemory<DisplayDx11>(&_blockAllocator, 1)) DisplayDx11(*renderer);

        display->_index = _displays.getCount();
        _displays.addLast() = display;

        return display;
    }

    void DisplayManager::unregisterDisplay(Display* display)
    {
        _displays.remove(display->_index);
    }

}
