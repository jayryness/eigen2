#include "Worklist.h"
#include "Renderer.h"

namespace eigen
{

    Worklist* Worklist::Create(Renderer* renderer, const RenderPlan* plan)
    {
        unsigned portRangeStart, portRangeEnd;
        plan->_ports.getRange(portRangeStart, portRangeEnd);

        unsigned depthSortCacheMask = std::max(plan->_count, portRangeEnd - portRangeStart);
        depthSortCacheMask *= 2;
        assert(depthSortCacheMask < 0x10000);
        depthSortCacheMask |= depthSortCacheMask >> 1;
        depthSortCacheMask |= depthSortCacheMask >> 2;
        depthSortCacheMask |= depthSortCacheMask >> 4;
        depthSortCacheMask |= depthSortCacheMask >> 8;

        uintptr_t sizeOfBatchLists = sizeof(BatchList) * (portRangeEnd - portRangeStart);
        uintptr_t sizeOfStages = (uintptr_t)plan->_end - (uintptr_t)plan->_start;
        uintptr_t sizeOfPerfSortCache = sizeof(SortCacheEntry) * (portRangeEnd - portRangeStart);
        uintptr_t sizeOfDepthSortCache = sizeof(SortCacheEntry) * (depthSortCacheMask+1);

        uintptr_t bytes = sizeof(Worklist) + sizeOfBatchLists + sizeOfStages + sizeOfPerfSortCache + sizeOfDepthSortCache + ChunkSize;
        Worklist* worklist = (Worklist*)renderer->scratchAlloc(bytes);
        assert(worklist != nullptr); // out of scratch memory TODO

        worklist->_renderer = renderer;
        worklist->_batchLists = (BatchList*)(worklist + 1) - portRangeStart;    // subtract start here instead of offsetting later
        worklist->_stages = (Stage*)(worklist->_batchLists + portRangeEnd);
        worklist->_stagesCount = plan->_count;
        worklist->_perfSortCache = (SortCacheEntry*)((int8_t*)worklist->_stages + sizeOfStages) - portRangeStart;
        worklist->_depthSortCache = worklist->_perfSortCache + portRangeEnd;
        worklist->_depthSortCacheMask = depthSortCacheMask;
        worklist->_buffer = (int8_t*)worklist->_depthSortCache + sizeOfDepthSortCache;
        worklist->_bufferEnd = worklist->_buffer + ChunkSize;
        worklist->_portRangeStart = portRangeStart;
        worklist->_portRangeEnd = portRangeEnd;

        //memcpy(worklist->_sortMasks, plan->_sortMasks, sizeof(worklist->_sortMasks));

        // copy stages into scratch memory
        memcpy(worklist->_stages, plan->_start, (int8_t*)plan->_end - (int8_t*)plan->_start);

        // clear batch slots
        memset(worklist->_batchLists + portRangeStart, 0, sizeOfBatchLists);

        // clear sort caches
        memset(worklist->_perfSortCache + portRangeStart, 0, sizeOfPerfSortCache);
        memset(worklist->_depthSortCache, 0, sizeOfDepthSortCache);

        return worklist;
    }

    void Worklist::commitBatch(Batch* batch, const RenderPort* port, float sortDepth)
    {
        // Batch is ignored if the pipeline doesn't listen to this port

        if (!port->getBit().intersects(_ports))
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

        BatchList& batchList = _batchLists[port->getIndex()];
        //entry->next = batchList.head.exchange(entry, std::memory_order_relaxed);  // [cb->next = head; head = cb;]
        entry->next = batchList.head;
        batchList.head = entry;
        batchList.count++;
    }

    void Worklist::finish()
    {
        _ports.clear();
        _renderer = nullptr;
    }

}
