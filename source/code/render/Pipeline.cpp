#include "Pipeline.h"

namespace eigen
{
    Pipeline::Pipeline()
    {
        _portSet.clear();
    }

    Pipeline::~Pipeline()
    {
        reset();
        Allocation::FreeMemory(_stages);
    }

    void Pipeline::reset()
    {
        PipelineManager* manager = (PipelineManager*)getManager();

        while (_count > 0)
        {
            Stage* last = _stages[--_count];
            switch (last->type)
            {
            case Stage::Type::Clear:
                manager->_clearStagePool.destroy((ClearStage*)last);
                break;
            case Stage::Type::Batch:
                manager->_batchStagePool.destroy((BatchStage*)last);
                break;
            case Stage::Type::Filter:
                manager->_filterStagePool.destroy((FilterStage*)last);
                break;
            }
            (RefPtr<TargetSet>&)last->targets = nullptr;
        }

        _portSet.clear();
    }

    void Pipeline::initialize(unsigned initialStageCapacity)
    {
        reset();

        if (initialStageCapacity > _capacity)
        {
            if (_stages)
            {
                Allocation::FreeMemory(_stages);
            }
            Allocator* allocator = getManager()->getAllocator();
            _stages = Allocation::AllocateMemory<Stage*>(allocator, initialStageCapacity);
            _capacity = initialStageCapacity;
        }
    }

    void PipelineManager::initialize(Allocator* allocator, unsigned initialCapacity)
    {
        Manager<Pipeline>::initialize<Pipeline_>(allocator, initialCapacity);

        _clearStagePool.initialize<ClearStage>(allocator, 16);
        _batchStagePool.initialize<BatchStage>(allocator, 64);
        _filterStagePool.initialize<FilterStage>(allocator, 32);
    }

}
