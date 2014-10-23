#pragma once

#include "core/Key.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderPort
    //
    // Routes batches to the desired pipeline stage(s)
    //

    enum { MaxRenderPorts = 80 };

    class RenderPort : public Key<RenderPort, MaxRenderPorts>
    {
    protected:

        RenderPort() {}
    };

}
