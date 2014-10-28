#pragma once

#include "Stage.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Pipeline
    //

    class Pipeline :            public RefCounted<Pipeline>
    {
    public:
                                friend class PipelineManager;

        void                    initialize(unsigned initialStageCapacity);

        void                    reset();
        void                    addStage(const Stage& stage);

        unsigned                getStageCount() const;
        const Stage&            getStage(unsigned index) const;

        PipelineManager*        getManager() const;

    protected:
                                friend void Delete<Pipeline>(Pipeline*);
                                friend class Worklist;

                                Pipeline();
                                ~Pipeline();

        void                    reserve(unsigned count);

        PipelineManager*        _manager    = 0;
        RenderPort::Set         _portSet;   
        Stage**                 _stages     = 0;
        unsigned                _count      = 0;
        unsigned                _capacity   = 0;
    };

    typedef RefPtr<Pipeline>    PipelinePtr;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Composer
    //
    // Helper class for building pipelines
    //

    class Composer
    {
    public:
                                Composer(Renderer& renderer, unsigned initialStageCapacity);
                                ~Composer();

        void                    reset();

        ClearStage&             addClear(TargetSet* targets);
        BatchStage&             addBatchStage(TargetSet* targets);
        FilterStage&            addFilter(TargetSet* targets);

        void                    saveToPipeline(Pipeline* pipeline);
        PipelinePtr             createPipeline(Renderer& renderer);

        unsigned                getStageCount() const;
        const Stage&            getStage(unsigned index) const;

    protected:

        void                    reserve(unsigned count);

        PipelineManager&        _manager;
        Stage**                 _stages = 0;
        unsigned                _count = 0;
        unsigned                _capacity = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // PipelineManager
    //

    class PipelineManager
    {
    public:

        void                    initialize(Allocator* allocator, unsigned initialCapacity);

        PipelinePtr             create();

    protected:
                                friend class Composer;
                                friend class Pipeline; // todo

        Allocator*              _allocator = 0;
        BlockAllocator          _pipelineAllocator;
        BlockAllocator          _clearStageAllocator;
        BlockAllocator          _batchStageAllocator;
        BlockAllocator          _filterStageAllocator;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    inline unsigned Pipeline::getStageCount() const
    {
        return _count;
    }

    inline const Stage& Pipeline::getStage(unsigned index) const
    {
        assert(index < _count);
        return *_stages[index];
    }

    inline PipelineManager* Pipeline::getManager() const
    {
        return _manager;
    }

    inline PipelinePtr PipelineManager::create()
    {
        Pipeline* pipeline = new(AllocateMemory<Pipeline>(&_pipelineAllocator, 1)) Pipeline();
        pipeline->_manager = this;
        return pipeline;
    }

}
