#include "../internal/RenderDispatch.h"
#include "RendererDx11.h"
#include "TargetSetDx11.h"

namespace eigen
{

    inline D3D11_CLEAR_FLAG TranslateClearFlags(ClearStage::Flags flags)
    {
        unsigned result = (unsigned)(flags & ClearStage::Flags::Depth) * D3D11_CLEAR_DEPTH / (unsigned)ClearStage::Flags::Depth;
        result |= (unsigned)(flags & ClearStage::Flags::Stencil) * D3D11_CLEAR_STENCIL / (unsigned)ClearStage::Flags::Stencil;
        return (D3D11_CLEAR_FLAG)result;
    }

    void RenderDispatch::submitStageJob(unsigned context, const StageJob& stageJob)
    {
        Renderer::PlatformDetails& plat = _renderer.getPlatformDetails();

        union
        {
            Stage*          stage;
            ClearStage*     clearStage;
            BatchStage*     batchStage;
            FilterStage*    filterStage;
        };

        stage = stageJob.stage;

        const TargetSetDx11* targets = (TargetSetDx11*)stage->targets;

        switch (stageJob.stage->type)
        {

        case Stage::Type::Clear:

            if (Any(clearStage->flags & ClearStage::Flags::Depth_Stencil) && targets->_depthStencilView.Get())
            {
                plat.immContext->ClearDepthStencilView(targets->_depthStencilView.Get(), TranslateClearFlags(clearStage->flags), clearStage->depth, (UINT8)clearStage->stencil);
            }

            for (unsigned i = 0; i < targets->getTextureCount() && targets->_targetViews[i].Get(); i++)
            {
                plat.immContext->ClearRenderTargetView(targets->_targetViews[i].Get(), (float*)(clearStage->colors + i));
            }

            return;

        case Stage::Type::Batch:


            return;

        case Stage::Type::Filter:


            return;
        }
    }

}
