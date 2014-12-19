#include "RendererDx11.h"
#include "RenderBufferDx11.h"

namespace eigen
{
    inline RenderBuffer::Bindings InferBindings(UINT bind, UINT misc)
    {
        RenderBuffer::Bindings result = RenderBuffer::Bindings::None;

        result |= (misc & D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS) ? RenderBuffer::Bindings::FunctionArgs : RenderBuffer::Bindings::None;
        result |= (bind & D3D11_BIND_UNORDERED_ACCESS)           ? RenderBuffer::Bindings::Scratch      : RenderBuffer::Bindings::None;
        result |= (bind & D3D11_BIND_VERTEX_BUFFER)              ? RenderBuffer::Bindings::Vertices     : RenderBuffer::Bindings::None;
        result |= (bind & D3D11_BIND_INDEX_BUFFER)               ? RenderBuffer::Bindings::Indices      : RenderBuffer::Bindings::None;
        result |= (bind & D3D11_BIND_RENDER_TARGET)              ? RenderBuffer::Bindings::RenderTarget : RenderBuffer::Bindings::None;

        return result;
    }

    inline RenderBuffer::Arena InferArena(D3D11_USAGE usage, UINT bind, UINT cpuAccess)
    {
        RenderBuffer::Arena result;

        if (usage == D3D11_USAGE_DYNAMIC || (cpuAccess & D3D11_CPU_ACCESS_READ))
        {
            result = RenderBuffer::Arena::Cooperative;
        }
        else if (bind & D3D11_BIND_CONSTANT_BUFFER)
        {
            result = RenderBuffer::Arena::ShaderVars;
        }
        else
        {
            result = RenderBuffer::Arena::GpuExclusive;
        }

        return result;
    }

    inline UINT TranslateBindFlags(RenderBuffer::Bindings bindings)
    {
        UINT result = D3D11_BIND_SHADER_RESOURCE;

        result |= Any(bindings & RenderBuffer::Bindings::Scratch)      ? D3D11_BIND_UNORDERED_ACCESS : 0;
        result |= Any(bindings & RenderBuffer::Bindings::Vertices)     ? D3D11_BIND_VERTEX_BUFFER    : 0;
        result |= Any(bindings & RenderBuffer::Bindings::Indices)      ? D3D11_BIND_INDEX_BUFFER     : 0;
        result |= Any(bindings & RenderBuffer::Bindings::RenderTarget) ? D3D11_BIND_RENDER_TARGET    : 0;

        return result;
    }

    inline UINT TranslateCpuAccessFlags(RenderBuffer::Arena arena)
    {
        UINT result = (arena != RenderBuffer::Arena::GpuExclusive) ? D3D11_CPU_ACCESS_WRITE : 0;
        return result;
    }

    inline UINT TranslateMiscFlags(RenderBuffer::Bindings bindings)
    {
        UINT result = Any(bindings & RenderBuffer::Bindings::Scratch) ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : 0;
        return result;
    }

    inline D3D11_USAGE TranslateUsage(RenderBuffer::Arena arena)
    {
        return (arena == RenderBuffer::Arena::GpuExclusive) ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
    }

    Error RenderBuffer::platformInit(const Config& config)
    {
        ComPtr<ID3D11Buffer> d3dResource;

        Renderer::PlatformDetails& plat = ((RenderBufferDx11*)this)->_renderer.getPlatformDetails();
        ID3D11Device* device = plat.device.Get();

        {
            D3D11_BUFFER_DESC desc;
            desc.ByteWidth              = config.elementStride * config.elementCount;
            desc.Usage                  = TranslateUsage(config.arena);
            desc.BindFlags              = TranslateBindFlags(config.bindings);
            desc.CPUAccessFlags         = TranslateCpuAccessFlags(config.arena);
            desc.MiscFlags              = TranslateMiscFlags(config.bindings);
            desc.StructureByteStride    = config.elementStride;

            HRESULT hr = device->CreateBuffer(&desc, nullptr, d3dResource.GetAddressOf());
            if (FAILED(hr))
            {
                EIGEN_RETURN_ERROR("Failed to create buffer, HRESULT = %d", hr);
            }
        }

        ((RenderBufferDx11*)this)->_d3dResource.Swap(d3dResource);

        EIGEN_RETURN_OK();
    }

    void RenderBuffer::platformDetach()
    {
        ((RenderBufferDx11*)this)->_d3dResource.Reset();
    }

}
