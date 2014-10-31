#pragma once

#include "Stage.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderPlan
    //
    // A sequence of Stages executed by the renderer to process a Worklist
    //

    class RenderPlan :          public RefCounted<RenderPlan>
    {
                                friend class RenderPlanManager;
    public:

        void                    reserve(uintptr_t bytes);   // not required, pre-allocates space for stages

        void                    reset();

        ClearStage&             addClearStage(TargetSetPtr targets);
        BatchStage&             addBatchStage(TargetSetPtr targets);
        FilterStage&            addFilterStage(TargetSetPtr targets);

        Error                   addStages(Stage** stages, unsigned stageCount);

        unsigned                getStageCount() const;

        Error                   validate();

        RenderPlanManager*      getManager() const;

    protected:
                                friend class Worklist;
                                friend void Delete<RenderPlan>(RenderPlan*);

                                RenderPlan();
                                ~RenderPlan();

        RenderPlanManager*      _manager        = 0;
        RenderPort::Set         _portSet;
        Stage*                  _start          = 0;
        Stage*                  _end            = 0;
        Stage*                  _validated      = 0;
        unsigned                _count          = 0;
        unsigned                _bytesCapacity  = 0;
    };

    typedef RefPtr<RenderPlan>  RenderPlanPtr;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderPlanManager
    //

    class RenderPlanManager
    {
    public:

        void                    initialize(Allocator* allocator, unsigned initialCapacity);

        RenderPlanPtr           create();

    protected:
                                friend class RenderPlan;

        Allocator*              _allocator = 0;
        BlockAllocator          _planAllocator;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    inline unsigned RenderPlan::getStageCount() const
    {
        return _count;
    }

    inline RenderPlanManager* RenderPlan::getManager() const
    {
        return _manager;
    }

    inline RenderPlanPtr RenderPlanManager::create()
    {
        RenderPlan* plan = new(AllocateMemory<RenderPlan>(&_planAllocator, 1)) RenderPlan();
        plan->_manager = this;
        return plan;
    }

    inline ClearStage& RenderPlan::addClearStage(TargetSetPtr targets) throw()
    {
        assert(targets.ptr != nullptr);
        reserve(sizeof(ClearStage));
        ClearStage* stage = new(_end) ClearStage;
        targets.swap((TargetSetPtr&)stage->targets);
        _end = stage + 1;
        _count++;
        return *stage;
    }

    inline BatchStage& RenderPlan::addBatchStage(TargetSetPtr targets) throw()
    {
        assert(targets.ptr != nullptr);
        reserve(sizeof(BatchStage));
        BatchStage* stage = new(_end) BatchStage;
        targets.swap((TargetSetPtr&)stage->targets);
        _end = stage + 1;
        _count++;
        return *stage;
    }

    inline FilterStage& RenderPlan::addFilterStage(TargetSetPtr targets) throw()
    {
        assert(targets.ptr != nullptr);
        reserve(sizeof(FilterStage));
        FilterStage* stage = new(_end) FilterStage;
        targets.swap((TargetSetPtr&)stage->targets);
        _end = stage + 1;
        _count++;
        return *stage;
    }

}
