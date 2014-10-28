#pragma once

#include "../Display.h"
#include "commonDx11.h"

namespace eigen
{

    class Renderer;

    class DisplayDx11 :         public Display
    {
    public:
                                DisplayDx11(Renderer&);
                                ~DisplayDx11();

        Renderer&               _renderer;
        ComPtr<IDXGISwapChain>  _swapChain;
    };

    inline DisplayDx11::DisplayDx11(Renderer& renderer)
        : _renderer(renderer)
    {
    }

    inline DisplayDx11::~DisplayDx11()
    {
    }

}
