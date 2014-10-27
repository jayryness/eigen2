#pragma once

#include "../TargetSet.h"
#include "TextureDx11.h"

namespace eigen
{
    class RendererDx11;

    class TargetSetDx11 :               public TargetSet
    {
    public:
                                        TargetSetDx11(Renderer& renderer);
                                        ~TargetSetDx11();

        Renderer&                       _renderer;

        ComPtr<ID3D11RenderTargetView>  _targetViews[MaxTextures];
        ComPtr<ID3D11DepthStencilView>  _depthStencilView;
        ComPtr<ID3D11DepthStencilView>  _readOnlyDepthStencilView;
    };

    inline TargetSetDx11::TargetSetDx11(Renderer& renderer)
        : _renderer(renderer)
    {
    }

    inline TargetSetDx11::~TargetSetDx11()
    {
    }


}
