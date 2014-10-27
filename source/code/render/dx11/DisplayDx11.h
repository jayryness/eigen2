#pragma once

#include "../Display.h"
#include "commonDx11.h"

namespace eigen
{

    class DisplayDx11 :         public Display
    {
    public:
                                DisplayDx11() {}
                                ~DisplayDx11();

        ComPtr<IDXGISwapChain>  _swapChain;
    };

    inline DisplayDx11::~DisplayDx11()
    {
    }

}
