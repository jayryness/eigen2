#pragma once

#include "../Texture.h"
#include "commonDx11.h"

namespace eigen
{

    class TextureDx11         : public Texture
    {
    public:
                                TextureDx11();
                               ~TextureDx11();

        void                    initWithResource(ID3D11Resource* d3dResource);
        void                    initWithTexture1D(ID3D11Texture1D* tex1d);
        void                    initWithTexture2D(ID3D11Texture2D* tex2d);
        void                    initWithTexture3D(ID3D11Texture3D* tex3d);

        ComPtr<ID3D11Resource> _d3dResource;
    };

    inline TextureDx11::TextureDx11()
    {
    }

    inline TextureDx11::~TextureDx11()
    {
    }

}
