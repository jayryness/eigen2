#pragma once

#include "core/Flag.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderPort
    //
    // Routes batches to the desired pipeline stage(s)
    //

    enum { MaxRenderPorts = 80 };

    class RenderPort : public Flag<RenderPort, MaxRenderPorts>
    {
    protected:

        RenderPort() {}
    };

}
