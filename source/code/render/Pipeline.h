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
                                friend class PipelineManager;
    public:

        struct Config
        {
            Stage**             stages;
            unsigned            stageCount;
        };

        Error                   initialize(Stage** stages, unsigned stageCount);

        unsigned                getStageCount() const;
        const Stage&            getStage(unsigned index) const;

        PipelineManager*        getManager() const;

    protected:
                                friend void Delete<Pipeline>(Pipeline*);
                                friend class Worklist;
                                friend class Composer;

                                Pipeline();
                                ~Pipeline();

        void                    reserve(unsigned count);

        PipelineManager*        _manager    = 0;
        RenderPort::Set         _portSet;   
        Stage**                 _stages     = 0;
        Stage*                  _start      = 0;
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

        void                    saveToPipeline(Pipeline* pipeline) const;
        PipelinePtr             createPipeline() const;

        unsigned                getStageCount() const;

    protected:

        void                    reserve(unsigned bytes);

        PipelineManager&        _manager;
        Stage*                  _start          = 0;
        Stage*                  _end            = 0;
        Stage**                 _stages         = 0;
        unsigned                _count          = 0;
        unsigned                _bytesCapacity  = 0;
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

    inline ClearStage& Composer::addClear(TargetSet* targets) throw()
    {
        reserve(sizeof(ClearStage));
        ClearStage* stage = new(_end) ClearStage;
        stage->targets = targets;
        _end = stage + 1;
        _count++;
        return *stage;
    }

    inline BatchStage& Composer::addBatchStage(TargetSet* targets) throw()
    {
        reserve(sizeof(BatchStage));
        BatchStage* stage = new(_end) BatchStage;
        stage->targets = targets;
        _end = stage + 1;
        _count++;
        return *stage;
    }

    inline FilterStage& Composer::addFilter(TargetSet* targets) throw()
    {
        reserve(sizeof(FilterStage));
        FilterStage* stage = new(_end) FilterStage;
        stage->targets = targets;
        _end = stage + 1;
        _count++;
        return *stage;
    }

    inline unsigned Composer::getStageCount() const
    {
        return _count;
    }

}
