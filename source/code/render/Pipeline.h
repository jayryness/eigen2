#pragma once

#include "TargetSet.h"
#include "RenderPort.h"
#include "core/Managed.h"
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

        ClearStage&             setColor(int target, const Float4& color);
        ClearStage&             setDepth(float depth);
        ClearStage&             setStencil(unsigned stencil);
        ClearStage&             setFlags(Flags flags);

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

        BatchStage&             setRenderPort(RenderPort* port);
        BatchStage&             setSortType(SortType sortType);

        RenderPort*             renderPort;
        SortType                sortType;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // FilterStage
    //

    struct FilterStage        : public Stage  // a.k.a. quad stage
    {
        // TODO
                                FilterStage();
        //Shader*               shader;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Pipeline
    //

    class Pipeline            : public Managed<Pipeline>
    {
    public:

        void                    initialize(unsigned initialStageCapacity);

        void                    reset();
        void                    addStage(const Stage& stage);

        unsigned                getStageCount() const;
        Stage*                  getStage(unsigned index) const;

        // refcounted guts with copy-on-write? (not just for api convenience, also for preventing pipeline mods during worklist population)
        //PipelinePtr           clone() const;

    protected:
                                friend class Worklist;

                                Pipeline();
                               ~Pipeline();
        void                    reserve(unsigned count);
        void                    unshare();

        RenderPort::Set        _portSet;
        Stage**                _stages      = 0;
        unsigned               _count       = 0;
        unsigned               _capacity    = 0;
    };

    typedef RefPtr<Pipeline>    PipelinePtr;

    class PipelineManager     : public Manager<Pipeline>
    {
    public:

        void                    initialize(Allocator* allocator, unsigned initialCapacity);

        PipelinePtr             create();

    protected:
                                friend class Pipeline;

        struct Pipeline_      : public Pipeline { ~Pipeline_() {} friend class PipelineManager; };  // ugh todo

        Pool                   _clearStagePool;
        Pool                   _batchStagePool;
        Pool                   _filterStagePool;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    EIGEN_DEFINE_ENUM_BIT_OPS(ClearStage::Flags)

    inline ClearStage::ClearStage()
    {
        type = Type::Clear;
    }

    inline BatchStage::BatchStage()
    {
        type = Type::Batch;
    }

    inline FilterStage::FilterStage()
    {
        type = Type::Filter;
    }

    inline void Pipeline::reserve(unsigned count) throw()
    {
        if (_count + count > _capacity)
        {
            Allocator* allocator = getManager()->getAllocator();
            _capacity += _count + count;
            Stage** stages = Allocation::AllocateMemory<Stage*>(allocator, _capacity);
            memcpy(stages, _stages, _count * sizeof(*stages));
            Allocation::FreeMemory(_stages);
        }
    }

    inline void Pipeline::addStage(const Stage& stage) throw()
    {
        if (!stage.targets)
        {
            assert(false);  // must set Stage targets
            return;
        }

        Stage* last = nullptr;
        PipelineManager* manager = (PipelineManager*)getManager();

        reserve(1);

        switch (stage.type)
        {
        case Stage::Type::Clear:
            last = (Stage*)new(manager->_clearStagePool.allocate()) ClearStage((ClearStage&)stage);
            break;
        case Stage::Type::Batch:
            last = (Stage*)new(manager->_batchStagePool.allocate()) BatchStage((BatchStage&)stage);
            _portSet |= ((BatchStage&)stage).renderPort->getBit();
            break;
        case Stage::Type::Filter:
            last = (Stage*)new(manager->_filterStagePool.allocate()) FilterStage((FilterStage&)stage);
            break;
        default:
            assert(false);
            return;
        }

        AddRef(last->targets);

        _stages[_count] = last;
        _count++;
    }

    inline unsigned Pipeline::getStageCount() const
    {
        return _count;
    }

    inline Stage* Pipeline::getStage(unsigned index) const
    {
        assert(index < _count);
        return _stages[index];
    }

    inline PipelinePtr PipelineManager::create()
    {
        return Manager<Pipeline>::create<Pipeline_>();
    }

}
