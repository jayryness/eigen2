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
            RenderPort*     port;
            float           sortDepth;
            //Batch         batch;    // shallow copy, variable # of paramblocks follow in memory
        };

        struct Slot
        {
            Item*           head;   // could make this atomic
            int             count;
            //Item**          sorted[BatchStage::SortType::Count];
        };
                            friend class Renderer;

                            Worklist() {}
                            Worklist(Renderer* renderer, const RenderPlan* plan);

        Renderer*           _renderer;
        const RenderPlan*   _plan;
        RenderPort::Set     _portSet;
        int8_t*             _buffer;
        int8_t*             _bufferEnd;
        Slot                _slots[MaxRenderPorts];  // todo compare vs single slot and sort
    };

    inline Worklist::Worklist(Renderer* renderer, const RenderPlan* plan)
        : _renderer(renderer)
        , _plan(plan)
        , _portSet(plan->_portSet)
        , _buffer(0)
        , _bufferEnd(0)
    {
        memset(_slots, 0, sizeof(_slots));
    }

}
