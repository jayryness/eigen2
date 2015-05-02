#pragma once

#include "core/memory.h"
#include "core/math.h"
#include "../RenderBin.h"
#include "../BatchQueue.h"  // TODO find better home for StageJob so this isn't needed

namespace eigen
{
    class BatchQueue;
    class Renderer;
    struct BatchStage;

    class RenderDispatch
    {
    public:
                                    struct Thread;

                                    RenderDispatch(Renderer& renderer);
                                    ~RenderDispatch();

        void                        initialize(Allocator* allocator, unsigned submissionThreads);

        void                        sync();
        void                        prepareWork(BatchQueue* head);
        void                        kick();

        void                        stop();

        Thread&                     getThread();

    private:
                                    struct SortJob;
                                    struct StageJob;

        void                        asyncRun();
        void                        addBatchQueueJobs(BatchQueue* batchQ, SortJob**& sortJobTail, StageJob*& stageJobEnd);
        SortJob*                    createSortJob(BatchQueue* batchQ, unsigned count);
        void                        submitStageJob(unsigned context, const StageJob& stageJob);

        Renderer&                   _renderer;

        BatchQueue*                 _head           = nullptr;

        SortJob*                    _sortJobHead    = nullptr;
        StageJob*                   _stageJobs      = nullptr;
        unsigned                    _stageJobCount  = 0;

        void*                       _threadSpace[6];
    };

    struct RenderDispatch::StageJob
    {
        Stage*                      stage;
        BatchQueue::SortBatch*      batches;
        unsigned                    batchStart;
        unsigned                    batchEnd;
    };

    inline RenderDispatch::Thread& RenderDispatch::getThread()
    {
        return (Thread&)_threadSpace;
    }

}
