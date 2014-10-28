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

    inline void Pipeline::reserve(unsigned count) throw()
    {
        if (_count + count > _capacity)
        {
            _capacity += _count + count;
            Stage** stages = AllocateMemory<Stage*>(_manager->_allocator, _capacity);
            memcpy(stages, _stages, _count * sizeof(*stages));
            FreeMemory(_stages);
        }
    }

    void Pipeline::addStage(const Stage& stage) throw()
    {
        if (!stage.targets)
        {
            assert(false);  // must set Stage targets
            return;
        }

        Stage* last = nullptr;

        reserve(1);

        switch (stage.type)
        {
        case Stage::Type::Clear:
            last = new(AllocateMemory<ClearStage>(&_manager->_clearStageAllocator, 1)) ClearStage((ClearStage&)stage);
            break;
        case Stage::Type::Batch:
            last = new(AllocateMemory<BatchStage>(&_manager->_batchStageAllocator, 1)) BatchStage((BatchStage&)stage);
            _portSet |= ((BatchStage&)stage).renderPort->getBit();
            break;
        case Stage::Type::Filter:
            last = new(AllocateMemory<FilterStage>(&_manager->_filterStageAllocator, 1)) FilterStage((FilterStage&)stage);
            break;
        default:
            assert(false);
            return;
        }

        AddRef(last->targets);

        _stages[_count] = last;
        _count++;
    }

}
