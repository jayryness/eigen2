#pragma once

#include "core/math.h"
#include "../RenderPort.h"

namespace eigen
{
    class Worklist;
    class Renderer;

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

        enum                        { SortCacheSize = StaticNextPow2<MaxRenderPorts>::Result };

        struct SortCacheEntry
        {
            unsigned                hash;
            unsigned                index;
        };

        void                        thread();

        Worklist*                   _head = nullptr;

        SortCacheEntry              _depthSortCache[SortCacheSize];

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
