#pragma once

#include "../TargetSet.h"
#include "TextureDx11.h"

namespace eigen
{

    class TargetSetDx11               : public TargetSet
    {
    public:
                                        TargetSetDx11();
                                       ~TargetSetDx11();

        ComPtr<ID3D11RenderTargetView> _targetViews[MaxTextures];
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilView> _readOnlyDepthStencilView;
    };

    inline TargetSetDx11::TargetSetDx11()
    {
    }

    inline TargetSetDx11::~TargetSetDx11()
    {
    }


}
