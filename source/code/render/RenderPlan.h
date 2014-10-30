#pragma once

#include "Stage.h"

namespace eigen
{
    class RenderPlanner;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderPlan
    //

    class RenderPlan :          public RefCounted<RenderPlan>
    {
                                friend class RenderPlanManager;
    public:

        Error                   initialize(Stage** stages, unsigned stageCount);
        Error                   initialize(const RenderPlanner& planner);

        unsigned                getStageCount() const;
        Stage*                  getFirstStage() const;
        Stage*                  getNextStage(Stage* prev) const;

        RenderPlanManager*      getManager() const;

    protected:
                                friend void Delete<RenderPlan>(RenderPlan*);
                                friend class Worklist;
                                friend class RenderPlanner;

                                RenderPlan();
                                ~RenderPlan();

        void                    reserve(unsigned count);

        RenderPlanManager*      _manager    = 0;
        RenderPort::Set         _portSet;   
        Stage*                  _start      = 0;
        Stage*                  _end        = 0;
        unsigned                _count      = 0;
    };

    typedef RefPtr<RenderPlan>  RenderPlanPtr;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderPlanner
    //
    // Helper class for building renderplans
    //

    class RenderPlanner
    {
    public:
                                RenderPlanner(Renderer& renderer, unsigned initialStageCapacity);
                                ~RenderPlanner();

        void                    reset();

        ClearStage&             addClear(TargetSetPtr targets);
        BatchStage&             addBatchStage(TargetSetPtr targets);
        FilterStage&            addFilter(TargetSetPtr targets);

        unsigned                getStageCount() const;

    protected:
                                friend class RenderPlan;

        void                    reserve(unsigned bytes);

        RenderPlanManager&      _manager;
        Stage*                  _start          = 0;
        Stage*                  _end            = 0;
        unsigned                _count          = 0;
        unsigned                _bytesCapacity  = 0;
    };

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
                                friend class RenderPlanner;
                                friend class RenderPlan; // todo

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

    inline ClearStage& RenderPlanner::addClear(TargetSetPtr targets) throw()
    {
        assert(targets.ptr != nullptr);
        reserve(sizeof(ClearStage));
        ClearStage* stage = new(_end) ClearStage;
        targets.swap((TargetSetPtr&)stage->targets);
        _end = stage + 1;
        _count++;
        return *stage;
    }

    inline BatchStage& RenderPlanner::addBatchStage(TargetSetPtr targets) throw()
    {
        assert(targets.ptr != nullptr);
        reserve(sizeof(BatchStage));
        BatchStage* stage = new(_end) BatchStage;
        targets.swap((TargetSetPtr&)stage->targets);
        _end = stage + 1;
        _count++;
        return *stage;
    }

    inline FilterStage& RenderPlanner::addFilter(TargetSetPtr targets) throw()
    {
        assert(targets.ptr != nullptr);
        reserve(sizeof(FilterStage));
        FilterStage* stage = new(_end) FilterStage;
        targets.swap((TargetSetPtr&)stage->targets);
        _end = stage + 1;
        _count++;
        return *stage;
    }

    inline unsigned RenderPlanner::getStageCount() const
    {
        return _count;
    }

}
