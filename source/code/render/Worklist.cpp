#include "Worklist.h"
#include "Renderer.h"

namespace eigen
{

    void Worklist::commitBatch(Batch* batch, RenderPort* port, float sortDepth)
    {
        // Batch is ignored if the pipeline doesn't listen to this port

        if (!port->getBit().intersects(_portSet))
        {
            return;
        }

        // Create committed batch in scratch memory

        unsigned bytes = sizeof(Item);// + dataCount*sizeof(StructData*);

        if (_buffer + bytes > _bufferEnd)
        {
            enum { chunkSize = 16 * 1024 };
            int8_t* p = _renderer->scratchAlloc(chunkSize);
            if (p == nullptr)
            {
                // error TODO
                return;
            }
            _buffer     = p;
            _bufferEnd  = p + chunkSize;
            assert(_buffer + bytes <= _bufferEnd);
        }

        Item* item = (Item*)_buffer;
        _buffer += bytes;

        item->next      = nullptr;
        item->port      = port;
        item->sortDepth = sortDepth;

        // Commit batch to list

        Slot& slot = _slots[port->getIndex()];
        //item->next = slot.head.exchange(item, std::memory_order_relaxed);  // [cb->next = head; head = cb;]
        item->next = slot.head;
        slot.head = item;
        slot.count++;
    }

    void Worklist::finish()
    {
        _portSet.clear();
        _renderer = nullptr;
    }

}
