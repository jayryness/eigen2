#include "../Renderer.h"
#include <thread>
#include <condition_variable>
#include <mutex>

namespace eigen
{
    struct RenderDispatch::Thread
    {
        static void Run(RenderDispatch* coordinator)
        {
            coordinator->asyncRun();
        }

        Thread(RenderDispatch* coordinator)
            : stopRequested(false)
            , mutex()
            , thread(Run, coordinator)
        {
        }

        bool                    stopRequested;
        std::mutex              mutex;
        std::thread             thread;
    };

    struct RenderDispatch::SortJob
    {
        SortJob*                next;
        BatchQueue*             batchQ;
        BatchStage::SortType    sortType;
        BatchQueue::CachedSort  cachedSort;

        void execute();
    };

    RenderDispatch::RenderDispatch(Renderer& renderer)
        : _renderer(renderer)
    {
        static_assert(sizeof(Thread) <= sizeof(_threadSpace), "Must increaase size of RenderDispatch::_threadSpace");
        new(&_threadSpace) Thread(this);
    }

    RenderDispatch::~RenderDispatch()
    {
        stop();
    }

    void RenderDispatch::initialize(Allocator* allocator, unsigned submissionThreads)
    {
    }

    void RenderDispatch::asyncRun()
    {
        Thread& thread = getThread();

        while (true)
        {
            std::unique_lock<std::mutex> lock(thread.mutex);

            if (thread.stopRequested)
                return;

            // TODO no concurrent sort job issuing yet
            for (SortJob* job = _sortJobHead; job; job = job->next)
            {
                job->execute();
            }

            // Issue batches

            for (unsigned i = 0; i < _stageJobCount; i++)
            {
                submitStageJob(0, _stageJobs[i]);
            }
        }
    }

    void RenderDispatch::sync()
    {
        Thread& thread = getThread();

        thread.mutex.lock();
        _head = nullptr;
    }

    inline RenderDispatch::SortJob* RenderDispatch::createSortJob(BatchQueue* batchQ, unsigned count)
    {
        unsigned bytes = sizeof(SortJob) + sizeof(BatchQueue::SortBatch) * count;
        SortJob* job = (SortJob*)_renderer.scratchAlloc(bytes);

        job->next = nullptr;
        job->batchQ = batchQ;
        job->cachedSort.count = count;
        job->cachedSort.batches = (BatchQueue::SortBatch*)(job + 1);

        return job;
    }

    void RenderDispatch::addBatchQueueJobs(BatchQueue* batchQ, SortJob**& sortJobTail, StageJob*& stageJobEnd)
    {
        unsigned submissionCost = 0;

        Stage* stage = batchQ->_stages;
        for (unsigned stageCount = batchQ->_stagesCount; stageCount > 0; stageCount--, stage = stage->advance())
        {
            stage->targets->_touch(_renderer.getFrameNumber());

            stageJobEnd->stage = stage;
            stageJobEnd->batches = nullptr;
            stageJobEnd->batchStart = 0;
            stageJobEnd->batchEnd = 0;

            BatchStage* batchStage;

            switch (stage->type)
            {
            case Stage::Type::Clear:
                stageJobEnd++;
                continue;
            case Stage::Type::Batch:
                batchStage = (BatchStage*)stage;
                break;
            case Stage::Type::Filter:
                submissionCost++;
                stageJobEnd++;
                continue;
            default:
                assert(false);  // batchQ is corrupt
                return;
            }

            bool isDepthSort = (batchStage->sortType == BatchStage::SortType::IncreasingDepth || batchStage->sortType == BatchStage::SortType::DecreasingDepth);

            unsigned count = 0;
            batchStage->attachedBins.forEach(
                [&](unsigned binIndex, const RenderBin::Set& )
                {
                    count += batchQ->_batchLists[binIndex].count;
                }
            );

            if (count == 0)
                continue;

            submissionCost += count;

            BatchQueue::SortCacheEntry& sortCacheEntry = isDepthSort ? batchQ->findCachedDepthSort(batchStage->attachedBins) : batchQ->findCachedPerfSort(batchStage->attachedBins);
            if (sortCacheEntry.cached == nullptr)
            {
                SortJob* job = createSortJob(batchQ, count);
                job->sortType = isDepthSort ? BatchStage::SortType::IncreasingDepth : BatchStage::SortType::Performance;
                job->cachedSort.binMask = batchStage->attachedBins;

                sortCacheEntry.cached = &job->cachedSort;

                *sortJobTail = job;
                sortJobTail = &job->next;
            }
            assert(sortCacheEntry.cached->binMask == batchStage->attachedBins && sortCacheEntry.cached->count == count);

            stageJobEnd->batches = sortCacheEntry.cached->batches;
            stageJobEnd->batchStart = 0;
            stageJobEnd->batchEnd = sortCacheEntry.cached->count;
            stageJobEnd++;
        }
    }

    void RenderDispatch::prepareWork(BatchQueue* head)
    {
        assert(_head == nullptr);
        _head = head;

        // Count total stages across all batchQs

        _stageJobCount = 0;
        for (BatchQueue* batchQ = _head; batchQ; batchQ = batchQ->_next)
        {
            _stageJobCount += batchQ->_stagesCount;
        }

        // Populate sort jobs and stage jobs

        _sortJobHead = nullptr;
        SortJob** sortJobTail = &_sortJobHead;

        _stageJobs = (StageJob*)_renderer.scratchAlloc(sizeof(StageJob) * _stageJobCount);
        StageJob* stageJobEnd = _stageJobs;

        for (BatchQueue* batchQ = _head; batchQ; batchQ = batchQ->_next)
        {
            addBatchQueueJobs(batchQ, sortJobTail, stageJobEnd);
        }
    }

    void RenderDispatch::kick()
    {
        Thread& thread = getThread();

        thread.mutex.unlock();
    }

    void RenderDispatch::stop()
    {
        Thread& thread = getThread();

        sync();

        thread.stopRequested = true;

        kick();

        if (thread.thread.joinable())   // TODO seems sketchy, fixes shutdown problem
            thread.thread.join();
    }

    void RenderDispatch::SortJob::execute()
    {
        assert(cachedSort.count > 0);

        unsigned count = 0;

        // Copy batches from slots into sort array
        cachedSort.binMask.forEach(
            [&](unsigned index, const RenderBin::Set&)
            {
                BatchQueue::BatchListEntry* entry = batchQ->_batchLists[index].head;
                for (; entry; entry = entry->next)
                {
                    assert(count < cachedSort.count);
                    if (sortType == BatchStage::SortType::Performance)
                    {
                        cachedSort.batches[count].sortKey = entry->performanceSortKey;
                    }
                    else
                    {
                        cachedSort.batches[count].sortKey = 0;
                        (float&)cachedSort.batches[count].sortKey = entry->sortDepth;
                    }
                    //cachedSort.batches[count].batch = &item->batch;   TODO
                    cachedSort.batches[count].batch = nullptr;
                    count++;
                }
            }
        );

        assert(count == cachedSort.count);

        std::sort(cachedSort.batches, cachedSort.batches + count);
    }

}
