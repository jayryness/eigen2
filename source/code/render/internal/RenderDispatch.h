#pragma once

#include "core/memory.h"
#include "core/math.h"
#include "../RenderPort.h"
#include "../Worklist.h"    // TODO find better home for StageJob so this isn't needed

namespace eigen
{
    class Worklist;
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
        void                        prepareWork(Worklist* head);
        void                        kick();

        void                        stop();

        Thread&                     getThread();

    private:
                                    struct SortJob;
                                    struct StageJob;

        void                        asyncRun();
        void                        addWorklistJobs(Worklist* worklist, SortJob**& sortJobTail, StageJob*& stageJobEnd);
        SortJob*                    createSortJob(Worklist* worklist, unsigned count);
        void                        submitStageJob(unsigned context, const StageJob& stageJob);

        Renderer&                   _renderer;

        Worklist*                   _head           = nullptr;

        SortJob*                    _sortJobHead    = nullptr;
        StageJob*                   _stageJobs      = nullptr;
        unsigned                    _stageJobCount  = 0;

        void*                       _threadSpace[6];
    };

    struct RenderDispatch::StageJob
    {
        Stage*                      stage;
        Worklist::SortBatch*        batches;
        unsigned                    batchStart;
        unsigned                    batchEnd;
    };

    inline RenderDispatch::Thread& RenderDispatch::getThread()
    {
        return (Thread&)_threadSpace;
    }

}
