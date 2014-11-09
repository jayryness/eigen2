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
        SortJob*                next;
        Worklist*               worklist;
        BatchStage::SortType    sortType;
        Worklist::CachedSort    cachedSort;

        void execute();
    };

    struct WorkCoordinator::StageJob
    {
        StageJob*               next;
        Stage*                  stage;
        unsigned                count;
        SortJob*                sortJob;
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

    void WorkCoordinator::addSortJob(Renderer& renderer, Worklist* worklist, BatchStage* batchStage, SortJob**& tail)
    {
        if (batchStage->sortType == BatchStage::SortType::Performance)
        {
            // For performance sort, slots are sorted individually, so multiple sort jobs are created

            unsigned cur, end;
            batchStage->ports.getRange(cur, end);
            RenderPort::Set curBit = batchStage->ports;
            curBit.clearExceptLsb();

            for (; cur < end; cur++, curBit<<=1)
            {
                if (!curBit.intersects(batchStage->ports) || !worklist->_slots[cur].count)
                    continue;

                unsigned hash = curBit.hash();

                hash ^= 0xabcdef12; // Ok, this sucks. Need to discriminate perf sort jobs from depth sort jobs. Whole system needs refactor TODO

                Worklist::SortCacheEntry& sortCacheEntry = worklist->findCachedSort(hash);
                if (sortCacheEntry.cached == nullptr)
                {
                    unsigned bytes = sizeof(SortJob) + sizeof(Worklist::SortBatch) * worklist->_slots[cur].count;
                    SortJob* job = (SortJob*)renderer.scratchAlloc(bytes);

                    job->next = nullptr;
                    job->sortType = BatchStage::SortType::Performance;
                    job->worklist = worklist;
                    job->cachedSort.count = worklist->_slots[cur].count;
                    job->cachedSort.ports = curBit;
                    job->cachedSort.batches = (Worklist::SortBatch*)(job + 1);

                    sortCacheEntry.cached = &job->cachedSort;

                    *tail = job;
                    tail = &job->next;
                }
                assert(sortCacheEntry.cached->ports == curBit && sortCacheEntry.cached->count == worklist->_slots[cur].count);
            }
        }
        else if (batchStage->sortType == BatchStage::SortType::IncreasingDepth || batchStage->sortType == BatchStage::SortType::DecreasingDepth)
        {
            // For depth sort, all participating slots are merged and sorted in one big array/job

            unsigned cur, end;
            batchStage->ports.getRange(cur, end);
            RenderPort::Set curBit = batchStage->ports;
            curBit.clearExceptLsb();

            unsigned count = 0;
            for (; cur < end; cur++, curBit <<= 1)
            {
                if (curBit.intersects(batchStage->ports))
                    count += worklist->_slots[cur].count;
            }

            if (count == 0)
                return;

            unsigned hash = batchStage->ports.hash();
            Worklist::SortCacheEntry& sortCacheEntry = worklist->findCachedSort(hash);
            if (sortCacheEntry.cached == nullptr)
            {
                unsigned bytes = sizeof(SortJob) + sizeof(Worklist::SortBatch) * count;
                SortJob* job = (SortJob*)renderer.scratchAlloc(bytes);

                job->next = nullptr;
                job->sortType = BatchStage::SortType::IncreasingDepth;  // decreasing depth is handled by submitting the batches back to front
                job->worklist = worklist;
                job->cachedSort.count = count;
                job->cachedSort.ports = batchStage->ports;
                job->cachedSort.batches = (Worklist::SortBatch*)(job + 1);

                sortCacheEntry.cached = &job->cachedSort;

                *tail = job;
                tail = &job->next;
            }
            assert(sortCacheEntry.cached->ports == batchStage->ports && sortCacheEntry.cached->count == count);
        }
        else
        {
            assert(false);
        }
    }

    void WorkCoordinator::prepareWork(Renderer& renderer, Worklist* head)
    {
        assert(_head == nullptr);
        _head = head;

        // Create sort jobs

        _sortJobs = nullptr;
        SortJob** sortJobTail = &_sortJobs;

        for (Worklist* worklist = _head; worklist; worklist = worklist->_next)
        {
            Stage* stage = worklist->_stages;
            for (unsigned count = worklist->_stagesCount; count > 0; count--)
            {
                if (stage->type == Stage::Type::Batch)
                {
                    BatchStage* batchStage = (BatchStage*)stage;
                    addSortJob(renderer, worklist, batchStage, sortJobTail);
                }
                stage = stage->advance();
            }
        }

        // Issue sort jobs

        // TODO no concurrent sort job issuing yet
        for (SortJob* job = _sortJobs; job; job = job->next)
        {
            job->execute();
        }

        // Wait for sort jobs to finish

        // Issue batches

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

    void WorkCoordinator::SortJob::execute()
    {
        assert(cachedSort.count > 0);

        unsigned cur, end;
        cachedSort.ports.getRange(cur, end);
        RenderPort::Set curBit = cachedSort.ports;
        curBit.clearExceptLsb();

        unsigned count = 0;
        // Copy batches from slots into sort array
        for (; cur < end; cur++, curBit <<= 1)
        {
            if (curBit.intersects(cachedSort.ports))
            {
                Worklist::Item* item = worklist->_slots[cur].head;
                for (; item; item = item->next)
                {
                    assert(count < cachedSort.count);
                    if (sortType == BatchStage::SortType::Performance)
                    {
                        cachedSort.batches[count].sortKey = item->performanceSortKey;
                    }
                    else
                    {
                        cachedSort.batches[count].sortKey = 0;
                        (float&)cachedSort.batches[count].sortKey = item->sortDepth;
                    }
                    //cachedSort.batches[count].batch = &item->batch;   TODO
                    cachedSort.batches[count].batch = nullptr;
                    count++;
                }
            }
        }
        assert(count == cachedSort.count);

        std::sort(cachedSort.batches, cachedSort.batches + count);
    }

}
