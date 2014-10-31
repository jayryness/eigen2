#pragma once

#include "TargetSet.h"
#include "RenderPort.h"
#include "core/types.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Stage
    //
    // A phase of the rendering pipeline, specifying the output targets and operations on them.
    //
    // There are three types:
    //
    // - ClearStage clears the targets
    // - BatchStage renders all batches received by a given RenderPort
    // - FilterStage invokes a shader
    //

    struct Stage
    {
        enum class Type
        {
            Unspecified = 0,
            Clear,
            Batch,
            Filter,
        };

        TargetSet*              targets = nullptr;
        Type                    type    = Type::Unspecified;

    protected:
                                Stage() {}
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // ClearStage
    //

    struct ClearStage : public Stage
    {
        enum class Flags
        {
            None                = 0,
            Color               = (1 << 0),
            Depth               = (1 << 1),
            Stencil             = (1 << 2),

            Color_Depth_Stencil = (Color | Depth | Stencil),
            Color_Depth         = (Color | Depth          ),
            Color_Stencil       = (Color |         Stencil),
            Depth_Stencil       = (        Depth | Stencil),
        };

                                ClearStage();

        Float4                  colors[TargetSet::MaxTextures];
        float                   depth   = 0.f;
        unsigned                stencil = 0;
        Flags                   flags   = Flags::None;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // BatchStage
    //

    struct BatchStage         : public Stage
    {
        enum SortType
        {
            Default           = 0,
            Performance       = Default,
            IncreasingDepth,
            DecreasingDepth,
            Count
        };
                                BatchStage();

        RenderPort*             renderPort;
        SortType                sortType;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // FilterStage
    //

    struct FilterStage        : public Stage
    {
        // TODO
                                FilterStage();
        //Shader*               shader;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    EIGEN_DEFINE_ENUM_BIT_OPS(ClearStage::Flags)

    inline ClearStage::ClearStage()
    {
        type = Type::Clear;
        memset(colors, 0, sizeof(colors));
    }

    inline BatchStage::BatchStage()
    {
        type = Type::Batch;
    }

    inline FilterStage::FilterStage()
    {
        type = Type::Filter;
    }

}
