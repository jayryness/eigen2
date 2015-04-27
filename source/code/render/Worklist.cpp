#include "Worklist.h"
#include "Renderer.h"

namespace eigen
{

    Worklist* Worklist::Create(Renderer* renderer, const RenderPlan* plan)
    {
        unsigned binRangeStart, binRangeEnd;
        plan->_binMask.getRange(binRangeStart, binRangeEnd);

        unsigned sortCacheMask = FloodBitsRight(plan->_count * 2);

        uintptr_t sizeOfBatchLists = sizeof(BatchList) * (binRangeEnd - binRangeStart);
        uintptr_t sizeOfStages = (uintptr_t)plan->_end - (uintptr_t)plan->_start;
        uintptr_t sizeOfPerfSortCache = sizeof(SortCacheEntry) * (sortCacheMask+1);
        uintptr_t sizeOfDepthSortCache = sizeof(SortCacheEntry) * (sortCacheMask+1);

        uintptr_t bytes = sizeof(Worklist) + sizeOfBatchLists + sizeOfStages + sizeOfPerfSortCache + sizeOfDepthSortCache + ChunkSize;
        Worklist* worklist = (Worklist*)renderer->scratchAlloc(bytes);
        assert(worklist != nullptr); // out of scratch memory TODO

        worklist->_renderer = renderer;
        worklist->_batchLists = (BatchList*)(worklist + 1) - binRangeStart;    // subtract start here instead of offsetting later
        worklist->_stages = (Stage*)(worklist->_batchLists + binRangeEnd);
        worklist->_perfSortCache = (SortCacheEntry*)((int8_t*)worklist->_stages + sizeOfStages);
        worklist->_depthSortCache = worklist->_perfSortCache + sortCacheMask+1;
        worklist->_buffer = (int8_t*)worklist->_depthSortCache + sizeOfDepthSortCache;
        worklist->_bufferEnd = worklist->_buffer + ChunkSize;
        worklist->_binMask = plan->_binMask;
        worklist->_stagesCount = plan->_count;
        worklist->_sortCacheMask = sortCacheMask;
        worklist->_binRangeStart = binRangeStart;
        worklist->_binRangeEnd = binRangeEnd;

        //memcpy(worklist->_sortMasks, plan->_sortMasks, sizeof(worklist->_sortMasks));

        // copy stages into scratch memory
        memcpy(worklist->_stages, plan->_start, (int8_t*)plan->_end - (int8_t*)plan->_start);

        // clear batch slots
        memset(worklist->_batchLists + binRangeStart, 0, sizeOfBatchLists);

        // clear sort caches
        memset(worklist->_perfSortCache, 0, sizeOfPerfSortCache);
        memset(worklist->_depthSortCache, 0, sizeOfDepthSortCache);

        return worklist;
    }

    void Worklist::commitBatch(RenderBatch* batch, const RenderBin* bin, float sortDepth)
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

    void Worklist::finish()
    {
        _binMask.clear();
        _renderer = nullptr;
    }

}
