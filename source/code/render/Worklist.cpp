#include "Worklist.h"
#include "Renderer.h"

namespace eigen
{

    Worklist* Worklist::Create(Renderer* renderer, const RenderPlan* plan)
    {
        unsigned portRangeStart, portRangeEnd;
        plan->_ports.getRange(portRangeStart, portRangeEnd);

        uintptr_t sizeOfSlots = sizeof(Slot) * (portRangeEnd - portRangeStart);
        uintptr_t sizeOfStages = (uintptr_t)plan->_end - (uintptr_t)plan->_start;

        uintptr_t bytes = sizeof(Worklist) + sizeOfSlots + sizeOfStages + ChunkSize;
        Worklist* worklist = (Worklist*)renderer->scratchAlloc(bytes);
        assert(worklist != nullptr); // out of scratch memory TODO

        worklist->_renderer = renderer;
        worklist->_slots = (Slot*)(worklist + 1) - portRangeStart;      // subtract start here instead of offsetting later
        worklist->_stages = (Stage*)(worklist->_slots + portRangeEnd);
        worklist->_buffer = (int8_t*)worklist->_stages + sizeOfStages;
        worklist->_bufferEnd = worklist->_buffer + ChunkSize;
        worklist->_portRangeStart = portRangeStart;
        worklist->_portRangeEnd = portRangeEnd;

        memcpy(worklist->_sortMasks, plan->_sortMasks, sizeof(worklist->_sortMasks));

        // copy stages into scratch memory
        memcpy(worklist->_stages, plan->_start, (int8_t*)plan->_end - (int8_t*)plan->_start);

        // clear batch slots
        memset(worklist->_slots + portRangeStart, 0, sizeOfSlots);

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
