#pragma once
#include <cstdint>

#include "RenderPort.h"
#include "RenderPlan.h"

namespace eigen
{

    class Renderer;
    struct Batch;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Worklist
    //
    // Accumulates all the batches (draw calls) to be issued via a RenderPlan.
    //
    // Use Renderer::openWorklist() to acquire one.
    //
    // TODO - Is it threadsafe, or one thread per worklist and merge downsteam?
    //

    class Worklist
    {
    public:

        void                commitBatch(Batch* batch, const RenderPort* port, float sortDepth);
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
            int             count;  // and this
            SortBatch*      performanceSorted;
        };

        enum
        {
                            ChunkSize = 16*1024,
        };
                            friend class Renderer;
                            friend class WorkCoordinator;

        static Worklist*    Create(Renderer* renderer, const RenderPlan* plan);

        Worklist*           _next           = nullptr;
        Renderer*           _renderer       = nullptr;
        Stage*              _stages         = nullptr;
        Stage*              _stagesEnd      = nullptr;
        Slot*               _slots          = nullptr;
        int8_t*             _buffer         = nullptr;
        int8_t*             _bufferEnd      = nullptr;
        unsigned            _portRangeStart = 0;
        unsigned            _portRangeEnd   = 0;
        RenderPort::Set     _ports;
        RenderPort::Set     _sortMasks[BatchStage::SortType::Count];
    };

}
