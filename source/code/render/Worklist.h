#pragma once
#include <cstdint>

#include "RenderPort.h"
#include "RenderPlan.h"

namespace eigen
{

    class Renderer;

    struct Batch;

    class Worklist
    {
    public:

        void                commitBatch(Batch* batch, RenderPort* port, float sortDepth);
        void                finish();

    protected:

        struct Item
        {
            Item*           next;
            float           sortDepth;
            unsigned        performanceSortKey;
            //Batch         batch;    // shallow copy, variable # of paramblocks follow in memory
        };

        struct SortBatch
        {
            uint64_t        sortKey;
            Batch*          batch;
        };

        struct Slot
        {
            Item*           head;   // could make this atomic
            int             count;
            SortBatch*      performanceSorted;
            SortBatch*      depthSorted;
        };
                            friend class Renderer;
                            friend class WorkCoordinator;

                            Worklist() {}
                            Worklist(Renderer* renderer, const RenderPlan* plan);

        Worklist*           _next;
        Renderer*           _renderer;
        Stage*              _stages;
        RenderPort::Set     _ports;
        RenderPort::Set     _sortMasks[BatchStage::SortType::Count];
        int8_t*             _buffer;
        int8_t*             _bufferEnd;
        Slot                _slots[MaxRenderPorts];  // todo compare vs single slot and sort. or: it would be easy enough to condense this to only the slots of active ports for the renderplan
    };

}
