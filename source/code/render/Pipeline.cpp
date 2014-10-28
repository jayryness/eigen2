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
        FreeMemory(_stages);
    }

    void Pipeline::reset()
    {
        while (_count > 0)
        {
            Stage* last = _stages[--_count];
            switch (last->type)
            {
            case Stage::Type::Clear:
                Delete((ClearStage*)last);
                break;
            case Stage::Type::Batch:
                Delete((BatchStage*)last);
                break;
            case Stage::Type::Filter:
                Delete((FilterStage*)last);
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
                FreeMemory(_stages);
            }
            _stages = AllocateMemory<Stage*>(_manager->_allocator, initialStageCapacity);
            _capacity = initialStageCapacity;
        }
    }

    void PipelineManager::initialize(Allocator* allocator, unsigned initialCapacity)
    {
        assert(_allocator == nullptr);  // already initialized
        _allocator = allocator;

        _pipelineAllocator.initialize(allocator, sizeof(Pipeline), initialCapacity);
        _clearStageAllocator.initialize(allocator, sizeof(ClearStage), 16);
        _batchStageAllocator.initialize(allocator, sizeof(BatchStage), 64);
        _filterStageAllocator.initialize(allocator, sizeof(FilterStage), 32);
    }

}
