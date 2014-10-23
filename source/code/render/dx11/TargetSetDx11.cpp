#include "TargetSetDx11.h"
#include "RendererDx11.h"

namespace eigen
{
    inline void ViewDescFromTexture(D3D11_RENDER_TARGET_VIEW_DESC& desc, const Texture::Config& config, const Texture::Slice& slice)
    {
        desc.Format = DXGI_FORMAT_UNKNOWN;
        if (config.depth > 0)
        {
            assert(config.arrayLength == 0);
            assert(config.depth >= slice.depthEnd);
            assert(config.multisampling == Texture::Multisampling::None);
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
            desc.Texture3D.MipSlice = slice.mip;
            desc.Texture3D.FirstWSlice = slice.depthStart;
            desc.Texture3D.WSize = slice.depthEnd - slice.depthStart;
        }
        else if (config.arrayLength > 0)
        {
            assert(config.arrayLength >= slice.arrayEnd);

            if (config.multisampling == Texture::Multisampling::None)
            {
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray.MipSlice = slice.mip;
                desc.Texture2DArray.FirstArraySlice = slice.arrayStart;
                desc.Texture2DArray.ArraySize = slice.arrayEnd - slice.arrayStart;
            }
            else
            {
                assert(slice.mip == 0);
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                desc.Texture2DMSArray.FirstArraySlice = slice.arrayStart;
                desc.Texture2DMSArray.ArraySize = slice.arrayEnd - slice.arrayStart;
            }
        }
        else
        {
            if (config.multisampling == Texture::Multisampling::None)
            {
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = slice.mip;
            }
            else
            {
                assert(slice.mip == 0);
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            }
        }
    }

    inline void ViewDescFromTexture(D3D11_DEPTH_STENCIL_VIEW_DESC& desc, const Texture::Config& config, const Texture::Slice& slice)
    {
        desc.Format = DXGI_FORMAT_UNKNOWN;
        if (config.depth > 0)
        {
            assert(false);
        }
        else if (config.arrayLength > 0)
        {
            assert(config.arrayLength >= slice.arrayEnd);

            if (config.multisampling == Texture::Multisampling::None)
            {
                desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                desc.Texture2DArray.MipSlice = slice.mip;
                desc.Texture2DArray.FirstArraySlice = slice.arrayStart;
                desc.Texture2DArray.ArraySize = slice.arrayEnd - slice.arrayStart;
            }
            else
            {
                assert(slice.mip == 0);
                desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                desc.Texture2DMSArray.FirstArraySlice = slice.arrayStart;
                desc.Texture2DMSArray.ArraySize = slice.arrayEnd - slice.arrayStart;
            }
        }
        else
        {
            if (config.multisampling == Texture::Multisampling::None)
            {
                desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = slice.mip;
            }
            else
            {
                assert(slice.mip == 0);
                desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            }
        }
        desc.Flags = 0;
    }

    Error TargetSet::platformInit(const Config& config)
    {
        Renderer& renderer = Renderer::From(_manager);
        ID3D11Device* device = renderer.getPlatformDetails().device.Get();
        auto plat = (TargetSetDx11*)this;

        if (config.zbuffer)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC desc;
            ViewDescFromTexture(desc, config.zbuffer->getConfig(), config.zbufferSlice);

            HRESULT hr = device->CreateDepthStencilView(((TextureDx11*)config.zbuffer)->_d3dResource.Get(), &desc, plat->_depthStencilView.GetAddressOf());
            if (FAILED(hr))
            {
                EIGEN_RETURN_ERROR("Failed to create DepthStencilView, HRESULT = %d", hr);
            }

            desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
            hr = device->CreateDepthStencilView(((TextureDx11*)config.zbuffer)->_d3dResource.Get(), &desc, plat->_readOnlyDepthStencilView.GetAddressOf());
            assert(hr == S_OK);
        }
        for (unsigned i = 0; i < _textureCount; i++)
        {
            D3D11_RENDER_TARGET_VIEW_DESC desc;
            ViewDescFromTexture(desc, config.textures[i]->getConfig(), config.slices[i]);
            HRESULT hr = device->CreateRenderTargetView(((TextureDx11*)config.textures[i])->_d3dResource.Get(), &desc, plat->_targetViews[i].GetAddressOf());
            if (FAILED(hr))
            {
                EIGEN_RETURN_ERROR("Failed to create RenderTargetView, HRESULT = %d", hr);
            }
        }

        EIGEN_RETURN_OK();
    }

    void TargetSet::platformDetach()
    {
        auto plat = (TargetSetDx11*)this;
        for (unsigned i = 0; i < _textureCount; i++)
        {
            plat->_targetViews[i].Reset();
        }
        plat->_depthStencilView.Reset();
        plat->_readOnlyDepthStencilView.Reset();
    }

}
