#pragma once

#include "core/SoftBitFlag.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderBin
    //
    // Routes batches to the desired pipeline stage(s)
    //

    enum { MaxRenderBins = 63 };

    class RenderBin : public SoftBitFlag<RenderBin, MaxRenderBins>
    {
    protected:

        RenderBin() {}
    };

    //struct RenderBinSetModifier
    //{
    //    void ForceBinOff(const RenderBin* bin);
    //    void ForceBinOn(const RenderBin* bin);

    //    RenderBin::Set  negativeOverrides;  // bitwise AND'd against set
    //    RenderBin::Set  positiveOverrides;  // bitwise OR'd into set
    //};

}
