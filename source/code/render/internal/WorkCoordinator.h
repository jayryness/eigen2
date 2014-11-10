#pragma once

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
                                    friend struct AsyncDetails;

                                    WorkCoordinator();
                                    ~WorkCoordinator();

        void                        sync();
        void                        prepareWork(Renderer& renderer, Worklist* head);
        void                        kick();

        void                        stop();

        AsyncDetails&               getAsyncDetails();

    private:
                                    struct SortJob;
                                    struct StageJob;

        void                        thread();
        void                        addSortJobs(Renderer& renderer, Worklist* worklist, SortJob**& tail);
        SortJob*                    createSortJob(Renderer& renderer, Worklist* worklist, unsigned count);

        Worklist*                   _head = nullptr;

        //PodArray<SortCacheEntry>    _sortCache;
        //unsigned                    _sortCacheMask  = SortCacheSize;

        SortJob*                    _sortJobs;
        StageJob*                   _stageJobs;

        void*                       _asyncDetails[6];
    };

    inline WorkCoordinator::~WorkCoordinator()
    {
        stop();
    }

    inline AsyncDetails& WorkCoordinator::getAsyncDetails()
    {
        return (AsyncDetails&)_asyncDetails;
    }

}
