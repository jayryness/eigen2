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

        uintptr_t sizeOfSlots = sizeof(Slot) * (portRangeEnd - portRangeStart);
        uintptr_t sizeOfStages = (uintptr_t)plan->_end - (uintptr_t)plan->_start;
        uintptr_t sizeOfPerfSortCache = sizeof(SortCacheEntry) * (portRangeEnd - portRangeStart);
        uintptr_t sizeOfDepthSortCache = sizeof(SortCacheEntry) * (depthSortCacheMask+1);

        uintptr_t bytes = sizeof(Worklist) + sizeOfSlots + sizeOfStages + sizeOfPerfSortCache + sizeOfDepthSortCache + ChunkSize;
        Worklist* worklist = (Worklist*)renderer->scratchAlloc(bytes);
        assert(worklist != nullptr); // out of scratch memory TODO

        worklist->_renderer = renderer;
        worklist->_slots = (Slot*)(worklist + 1) - portRangeStart;      // subtract start here instead of offsetting later
        worklist->_stages = (Stage*)(worklist->_slots + portRangeEnd);
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
        memset(worklist->_slots + portRangeStart, 0, sizeOfSlots);

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

        unsigned bytes = sizeof(Item);// + dataCount*sizeof(StructData*);

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

        Item* item = (Item*)_buffer;
        _buffer += bytes;

        item->next      = nullptr;
        item->sortDepth = sortDepth;
        item->performanceSortKey = 0;//batch->shaderId

        // Commit batch to list

        Slot& slot = _slots[port->getIndex()];
        //item->next = slot.head.exchange(item, std::memory_order_relaxed);  // [cb->next = head; head = cb;]
        item->next = slot.head;
        slot.head = item;
        slot.count++;
    }

    void Worklist::finish()
    {
        _ports.clear();
        _renderer = nullptr;
    }

}
