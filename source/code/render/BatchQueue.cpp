#include "BatchQueue.h"
#include "Renderer.h"

namespace eigen
{

    BatchQueue* BatchQueue::Create(Renderer* renderer, const RenderPlan* plan)
    {
        unsigned binRangeStart, binRangeEnd;
        plan->_binMask.getRange(binRangeStart, binRangeEnd);

        unsigned sortCacheMask = FloodBitsRight(plan->_count * 2);

        uintptr_t sizeOfBatchLists = sizeof(BatchList) * (binRangeEnd - binRangeStart);
        uintptr_t sizeOfStages = (uintptr_t)plan->_end - (uintptr_t)plan->_start;
        uintptr_t sizeOfPerfSortCache = sizeof(SortCacheEntry) * (sortCacheMask+1);
        uintptr_t sizeOfDepthSortCache = sizeof(SortCacheEntry) * (sortCacheMask+1);

        uintptr_t bytes = sizeof(BatchQueue) + sizeOfBatchLists + sizeOfStages + sizeOfPerfSortCache + sizeOfDepthSortCache + ChunkSize;
        BatchQueue* batchQ = (BatchQueue*)renderer->scratchAlloc(bytes);
        assert(batchQ != nullptr); // out of scratch memory TODO

        batchQ->_renderer = renderer;
        batchQ->_batchLists = (BatchList*)(batchQ + 1) - binRangeStart;    // subtract start here instead of offsetting later
        batchQ->_stages = (Stage*)(batchQ->_batchLists + binRangeEnd);
        batchQ->_perfSortCache = (SortCacheEntry*)((int8_t*)batchQ->_stages + sizeOfStages);
        batchQ->_depthSortCache = batchQ->_perfSortCache + sortCacheMask+1;
        batchQ->_buffer = (int8_t*)batchQ->_depthSortCache + sizeOfDepthSortCache;
        batchQ->_bufferEnd = batchQ->_buffer + ChunkSize;
        batchQ->_binMask = plan->_binMask;
        batchQ->_stagesCount = plan->_count;
        batchQ->_sortCacheMask = sortCacheMask;
        batchQ->_binRangeStart = binRangeStart;
        batchQ->_binRangeEnd = binRangeEnd;

        //memcpy(batchQ->_sortMasks, plan->_sortMasks, sizeof(batchQ->_sortMasks));

        // copy stages into scratch memory
        memcpy(batchQ->_stages, plan->_start, (int8_t*)plan->_end - (int8_t*)plan->_start);

        // clear batch slots
        memset(batchQ->_batchLists + binRangeStart, 0, sizeOfBatchLists);

        // clear sort caches
        memset(batchQ->_perfSortCache, 0, sizeOfPerfSortCache);
        memset(batchQ->_depthSortCache, 0, sizeOfDepthSortCache);

        return batchQ;
    }

    void BatchQueue::commitBatch(RenderBatch* batch, const RenderBin* bin, float sortDepth)
    {
        // Batch is ignored if the pipeline doesn't reference this bin

        if (!bin->getBit().intersects(_binMask))
        {
            return;
        }

        // Create committed batch in scratch memory

        unsigned bytes = sizeof(BatchListEntry);// + dataCount*sizeof(StructData*);

        if (_buffer + bytes > _bufferEnd)
        {
            int8_t* p = _renderer->scratchAlloc(ChunkSize);
            if (p == nullptr)
            {
                // error TODO
                return;
            }
            _buffer     = p;
            _bufferEnd  = p + ChunkSize;
            assert(_buffer + bytes <= _bufferEnd);
        }

        BatchListEntry* entry = (BatchListEntry*)_buffer;
        _buffer += bytes;

        entry->next                 = nullptr;
        entry->sortDepth            = sortDepth;
        entry->performanceSortKey   = 0;//batch->shaderId

        // Commit batch to list

        BatchList& batchList = _batchLists[bin->getPosition()];
        //entry->next = batchList.head.exchange(entry, std::memory_order_relaxed);  // [cb->next = head; head = cb;]
        entry->next = batchList.head;
        batchList.head = entry;
        batchList.count++;
    }

    void BatchQueue::finish()
    {
        _binMask.clear();
        _renderer = nullptr;
    }

}
