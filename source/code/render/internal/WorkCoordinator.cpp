#include "WorkCoordinator.h"
#include "../Renderer.h"
#include <thread>
#include <condition_variable>
#include <mutex>

namespace eigen
{
    struct AsyncDetails
    {
        static void Run(WorkCoordinator* coordinator)
        {
            coordinator->thread();
        }

        AsyncDetails(WorkCoordinator* coordinator)
            : stopRequested(false)
            , mutex()
            , thread(Run, coordinator)
        {
        }

        bool                    stopRequested;
        std::mutex              mutex;
        std::thread             thread;
    };

    struct WorkCoordinator::SortJob
    {
        SortJob*                next            = nullptr;
        Worklist*               worklist;
        RenderPort::Set         portSet;
        unsigned                count;
        Worklist::SortBatch*    depthSortedBatches;
        BatchStage::SortType    sortType;
    };

    struct WorkCoordinator::StageJob
    {
        StageJob*               next            = nullptr;
        Stage*                  stage;
        unsigned                count;
        Worklist::SortBatch*    sortedBatches   = nullptr;
    };

    WorkCoordinator::WorkCoordinator()
    {
        static_assert(sizeof(AsyncDetails) <= sizeof(_asyncDetails), "Must increaase size of WorkCoordinator::_asyncDetails");
        new(&_asyncDetails) AsyncDetails(this);
    }

    void WorkCoordinator::thread()
    {
        AsyncDetails& asyncDetails = getAsyncDetails();

        while (true)
        {
            std::unique_lock<std::mutex> lock(asyncDetails.mutex);

            if (asyncDetails.stopRequested)
                return;

            // do work here
        }
    }

    void WorkCoordinator::sync()
    {
        AsyncDetails& asyncDetails = getAsyncDetails();

        asyncDetails.mutex.lock();
        _head = nullptr;
    }

    void WorkCoordinator::prepareWork(Renderer& renderer, Worklist* head)
    {
        assert(_head == nullptr);
        _head = head;

        // Allocate memory for all the sorts

        _sortJobs = nullptr;
        SortJob** sortJobTail = &_sortJobs;

        for (Worklist* cur = _head; cur; cur = cur->_next)
        {
            RenderPort::Set portBit;
            portBit.set(cur->_portRangeStart, true);
            for (unsigned portIndex = cur->_portRangeStart; portIndex < cur->_portRangeEnd; portIndex++, portBit <<= 1)
            {
                Worklist::Slot* slot = cur->_slots + portIndex;
                if (slot->count == 0)
                    continue;

                bool performanceSort = portBit.intersects(cur->_sortMasks[BatchStage::SortType::Performance]);
                bool depthSort = portBit.intersects(cur->_sortMasks[BatchStage::SortType::IncreasingDepth]);
                depthSort |= portBit.intersects(cur->_sortMasks[BatchStage::SortType::DecreasingDepth]);

                unsigned bytes = /*sizeof(SortJob) + */sizeof(Worklist::SortBatch) * slot->count;
                if (performanceSort)
                {
                    //SortJob* sortJob = (SortJob*)renderer.scratchAlloc(bytes);
                    //sortJob->batches = (Worklist::SortBatch*)(sortJob + 1);
                    //sortJob->count = slot->count;
                    //sortJob->sortType = BatchStage::SortType::Performance;

                    //*sortJobTail = sortJob;
                    //sortJobTail = &sortJob->next;
                    slot->performanceSorted = (Worklist::SortBatch*)renderer.scratchAlloc(bytes);
                }
            }

            for (Stage* stage = cur->_stages; stage < cur->_stagesEnd; stage = stage->advance())
            {
                if (stage->type == Stage::Type::Batch)
                {
                    BatchStage* batchStage = (BatchStage*)stage;
                    if (batchStage->sortType == BatchStage::SortType::DecreasingDepth || batchStage->sortType == BatchStage::SortType::IncreasingDepth)
                    {
                        // depth sort job
                    }
                }
            }
        }
    }

    void WorkCoordinator::kick()
    {
        AsyncDetails& asyncDetails = getAsyncDetails();

        asyncDetails.mutex.unlock();
    }

    void WorkCoordinator::stop()
    {
        AsyncDetails& asyncDetails = getAsyncDetails();

        sync();

        asyncDetails.stopRequested = true;

        kick();

        asyncDetails.thread.join();
    }

}
