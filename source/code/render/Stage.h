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

        Stage*                  advance() const;

        Type                    type    = Type::Unspecified;
        TargetSet*              targets = nullptr;

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
            Performance       = 0,
            IncreasingDepth,
            DecreasingDepth,
            Count
        };
                                BatchStage();

        void                    addPort(const RenderPort* port);

        RenderPort::Set         ports;
        SortType                sortType    = SortType::Performance;
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

    inline void BatchStage::addPort(const RenderPort* port)
    {
        ports |= port->getBit();
    }

    inline FilterStage::FilterStage()
    {
        type = Type::Filter;
    }

    inline Stage* Stage::advance() const
    {
        switch (type)
        {
        case Type::Clear:   return (ClearStage*)this + 1;
        case Type::Batch:   return (BatchStage*)this + 1;
        case Type::Filter:  return (FilterStage*)this + 1;
        }
        return nullptr;
    }
}
