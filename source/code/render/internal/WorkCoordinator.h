#pragma once

#include "core/memory.h"
#include "core/math.h"
#include "../RenderPort.h"

namespace eigen
{
    class Worklist;
    class Renderer;
    struct BatchStage;

    class WorkCoordinator
    {
    public:
                                    struct Thread;

                                    WorkCoordinator(Renderer& renderer);
                                    ~WorkCoordinator();

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

        Renderer&                   _renderer;

        Worklist*                   _head           = nullptr;

        SortJob*                    _sortJobHead    = nullptr;
        StageJob*                   _stageJobs      = nullptr;
        unsigned                    _stageJobCount  = 0;

        void*                       _thread[6];
    };

    inline WorkCoordinator::Thread& WorkCoordinator::getThread()
    {
        return (Thread&)_thread;
    }

}
