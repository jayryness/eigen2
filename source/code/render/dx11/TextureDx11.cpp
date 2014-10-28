#include "RendererDx11.h"
#include "TextureDx11.h"
#include "FormatDx11.h"

namespace eigen
{
    inline Texture::Usage TranslateUsage(UINT bind, UINT cpuAccess)
    {
        Texture::Usage usage = Texture::Usage::Static;

        if (bind & (D3D11_BIND_RENDER_TARGET|D3D11_BIND_DEPTH_STENCIL))
        {
            usage = Texture::Usage::RenderTarget;
        }
        else if (cpuAccess & D3D11_CPU_ACCESS_WRITE)
        {
            usage = Texture::Usage::Dynamic;
        }

        return usage;
    }

    inline Texture::Flags TranslateFlags(UINT misc)
    {
        Texture::Flags flags = misc & D3D11_RESOURCE_MISC_TEXTURECUBE ? Texture::Flags::CubeMap  : Texture::Flags::None;
        return flags;
    }

    inline UINT TranslateBindFlags(Texture::Usage usage, Format format)
    {
        UINT bind = D3D11_BIND_SHADER_RESOURCE;

        if (usage == Texture::Usage::RenderTarget)
        {
            bind |= D3D11_BIND_RENDER_TARGET;
            bind |= IsDepthFormat(format) ? D3D11_BIND_DEPTH_STENCIL : 0;
        }

        return bind;
    }

    inline UINT TranslateCpuAccessFlags(Texture::Usage usage)
    {
        UINT cpuAccess = usage == Texture::Usage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
        return cpuAccess;
    }

    inline UINT TranslateMiscFlags(Texture::Flags flags)
    {
        return Any(flags & Texture::Flags::CubeMap) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
    }

    inline D3D11_USAGE TranslateUsage(Texture::Usage usage)
    {
        return usage == Texture::Usage::Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    }

    Error Texture::platformInit(const Config& config)
    {
        ComPtr<ID3D11Resource> d3dResource;

        Renderer::PlatformDetails& plat = ((TextureDx11*)this)->_renderer.getPlatformDetails();
        ID3D11Device* device = plat.device.Get();

        if (config.depth > 0)
        {
            D3D11_TEXTURE3D_DESC desc;
            desc.Width          = config.width;
            desc.Height         = config.height;
            desc.Depth          = config.depth;
            desc.MipLevels      = config.lastMip+1;
            desc.Format         = TranslateFormat(config.format);
            desc.BindFlags      = TranslateBindFlags(config.usage, config.format);
            desc.CPUAccessFlags = TranslateCpuAccessFlags(config.usage);
            desc.MiscFlags      = TranslateMiscFlags(config.flags);
            desc.Usage          = TranslateUsage(config.usage);

            HRESULT hr = device->CreateTexture3D(&desc, nullptr, (ID3D11Texture3D**) d3dResource.GetAddressOf());
            if (FAILED(hr))
            {
                EIGEN_RETURN_ERROR("Failed to create volume texture, HRESULT = %d", hr);
            }
        }
        else if (config.height > 0)
        {
            D3D11_TEXTURE2D_DESC desc;
            desc.Width              = config.width;
            desc.Height             = config.height;
            desc.MipLevels          = config.lastMip+1;
            desc.ArraySize          = config.arrayLength > 0 ? config.arrayLength : 1;
            desc.Format             = TranslateFormat(config.format);
            desc.SampleDesc.Count   = (UINT)1 << (UINT)config.multisampling;
            desc.SampleDesc.Quality = 0;
            desc.BindFlags          = TranslateBindFlags(config.usage, config.format);
            desc.CPUAccessFlags     = TranslateCpuAccessFlags(config.usage);
            desc.MiscFlags          = TranslateMiscFlags(config.flags);
            desc.Usage              = TranslateUsage(config.usage);

            HRESULT hr = device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**) d3dResource.GetAddressOf());
            if (FAILED(hr))
            {
                EIGEN_RETURN_ERROR("Failed to create texture, HRESULT = %d", hr);
            }
        }
        else
        {
            D3D11_TEXTURE1D_DESC desc;
            desc.Width          = config.width;
            desc.MipLevels      = config.lastMip+1;
            desc.ArraySize      = config.arrayLength > 0 ? config.arrayLength : 1;
            desc.Format         = TranslateFormat(config.format);
            desc.BindFlags      = TranslateBindFlags(config.usage, config.format);
            desc.CPUAccessFlags = TranslateCpuAccessFlags(config.usage);
            desc.MiscFlags      = TranslateMiscFlags(config.flags);
            desc.Usage          = TranslateUsage(config.usage);

            HRESULT hr = device->CreateTexture1D(&desc, nullptr, (ID3D11Texture1D**) d3dResource.GetAddressOf());
            if (FAILED(hr))
            {
                EIGEN_RETURN_ERROR("Failed to create 1D texture, HRESULT = %d", hr);
            }
        }

        ((TextureDx11*)this)->_d3dResource.Swap(d3dResource);

        EIGEN_RETURN_OK();
    }

    void Texture::platformDetach()
    {
        ((TextureDx11*)this)->_d3dResource.Reset();
    }

    void TextureDx11::initWithResource(ID3D11Resource* d3dResource)
    {
        D3D11_RESOURCE_DIMENSION dimension;
        d3dResource->GetType(&dimension);

        switch (dimension)
        {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            initWithTexture1D((ID3D11Texture1D*)d3dResource);
            break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            initWithTexture2D((ID3D11Texture2D*)d3dResource);
            break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            initWithTexture3D((ID3D11Texture3D*)d3dResource);
            break;

        default:
            assert(false);
            break;
        }
    }

    void TextureDx11::initWithTexture1D(ID3D11Texture1D* tex1d)
    {
        D3D11_TEXTURE1D_DESC desc;
        tex1d->GetDesc(&desc);

        _config = Config();
        _config.format          = TranslateFormat(desc.Format);
        _config.usage           = TranslateUsage(desc.BindFlags, desc.CPUAccessFlags);
        _config.flags           = TranslateFlags(desc.MiscFlags);
        _config.lastMip         = desc.MipLevels - 1;
        _config.width           = desc.Width;
        _config.arrayLength     = desc.ArraySize;

        _d3dResource = tex1d;
    }

    void TextureDx11::initWithTexture2D(ID3D11Texture2D* tex2d)
    {
        D3D11_TEXTURE2D_DESC desc;
        tex2d->GetDesc(&desc);

        _config = Config();
        _config.format          = TranslateFormat(desc.Format);
        _config.usage           = TranslateUsage(desc.BindFlags, desc.CPUAccessFlags);
        _config.flags           = TranslateFlags(desc.MiscFlags);
        _config.lastMip         = desc.MipLevels - 1;
        _config.width           = desc.Width;
        _config.height          = desc.Height;
        _config.arrayLength     = desc.ArraySize;

        _d3dResource = tex2d;
    }

    void TextureDx11::initWithTexture3D(ID3D11Texture3D* tex3d)
    {
        D3D11_TEXTURE3D_DESC desc;
        tex3d->GetDesc(&desc);

        _config = Config();
        _config.format          = TranslateFormat(desc.Format);
        _config.usage           = TranslateUsage(desc.BindFlags, desc.CPUAccessFlags);
        _config.flags           = TranslateFlags(desc.MiscFlags);
        _config.lastMip         = desc.MipLevels - 1;
        _config.width           = desc.Width;
        _config.height          = desc.Height;
        _config.depth           = desc.Depth;

        _d3dResource = tex3d;
    }

}
