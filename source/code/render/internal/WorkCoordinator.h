#pragma once

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

        void                        thread();

        Worklist*                   _head = nullptr;

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
