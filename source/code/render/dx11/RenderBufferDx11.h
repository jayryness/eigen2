#pragma once

#include "../RenderBuffer.h"
#include "commonDx11.h"

namespace eigen
{

    class Renderer;

    class RenderBufferDx11 :    public RenderBuffer
    {
    public:
                                RenderBufferDx11(Renderer&);
                                ~RenderBufferDx11();

        Renderer&               _renderer;
        ComPtr<ID3D11Buffer>    _d3dResource;
    };

    inline RenderBufferDx11::RenderBufferDx11(Renderer& renderer)
        : _renderer(renderer)
    {
    }

    inline RenderBufferDx11::~RenderBufferDx11()
    {
    }

}
