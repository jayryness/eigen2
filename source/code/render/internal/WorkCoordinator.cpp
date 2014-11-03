#include "WorkCoordinator.h"
#include "../Renderer.h"

namespace eigen
{

    WorkCoordinator::WorkCoordinator(Renderer& renderer, Worklist* head)
        : _head(head)
    {
        // Allocate memory for all the sorts

        for (Worklist* cur = _head; cur; cur = cur->_next)
        {
            RenderPort::Set portBit;
            portBit.set(0, true);
            for (Worklist::Slot* slot = cur->_slots; slot < cur->_slots+MaxRenderPorts; slot++, portBit<<=1)
            {
                if (slot->count == 0)
                    continue;

                bool performanceSort    = portBit.intersects(cur->_sortMasks[BatchStage::SortType::Performance]);
                bool depthSort          = portBit.intersects(cur->_sortMasks[BatchStage::SortType::IncreasingDepth]);
                depthSort              |= portBit.intersects(cur->_sortMasks[BatchStage::SortType::DecreasingDepth]);

                unsigned bytes = sizeof(Worklist::SortBatch) * slot->count;
                if (performanceSort)
                {
                    slot->performanceSorted = (Worklist::SortBatch*)renderer.scratchAlloc(bytes);
                }
                if (depthSort)
                {
                    slot->depthSorted = (Worklist::SortBatch*)renderer.scratchAlloc(bytes);
                }
            }
        }
    }

}
