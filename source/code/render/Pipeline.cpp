#include "Pipeline.h"
#include "Renderer.h"

namespace eigen
{
    Pipeline::Pipeline()
    {
        _portSet.clear();
    }

    Pipeline::~Pipeline()
    {
        for (int i = -(int)_count; i < 0; i++)
        {
            ReleaseRef(_stages[i]->targets);
        }
        FreeMemory(_start);
    }

    Error Pipeline::initialize(Stage** stages, unsigned stageCount)
    {
        assert(_stages == nullptr);     // already initialized

        unsigned bytes = 0;
        for (unsigned i = 0; i < stageCount; i++)
        {
            if (stages[i]->targets == nullptr)
            {
                EIGEN_RETURN_ERROR("Stage %d has invalid targets", (long)i);
            }
            switch(stages[i]->type)
            {
            case Stage::Type::Clear:
                bytes += sizeof(ClearStage);
                break;
            case Stage::Type::Batch:
                if (((BatchStage*)stages[i])->renderPort == nullptr)
                {
                    EIGEN_RETURN_ERROR("Stage %d has null render port", (long)i);
                }
                bytes += sizeof(BatchStage);
                break;
            case Stage::Type::Filter:
                bytes += sizeof(FilterStage);
                break;
            default:
                EIGEN_RETURN_ERROR("Invalid stage type at location %d", (long)i);
            }
        }

        bytes += stageCount*sizeof(Stage*);
        _start = (Stage*)AllocateMemory<char>(_manager->_allocator, bytes);
        Allocation::From(_start)->_metadata = (void*)1; // pipeline uses allocation metadata as refcount
        _stages = (Stage**)((char*)_start + bytes);
        _count = stageCount;

        Stage* last = _start;
        for (int i = -(int)_count; i < 0; i++)
        {
            AddRef(last->targets);
            _stages[-i] = last;
            switch(last->type)
            {
            case Stage::Type::Clear:
                last = (ClearStage*)last + 1;
                break;
            case Stage::Type::Batch:
                _portSet |= ((BatchStage*)last)->renderPort->getBit();
                last = (BatchStage*)last + 1;
                break;
            case Stage::Type::Filter:
                last = (FilterStage*)last + 1;
                break;
            default:
                break;
            }
        }

        EIGEN_RETURN_OK();
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

    Composer::Composer(Renderer& renderer, unsigned initialStageCapacity)
        : _manager(renderer.getPipelineManager())
    {
        _bytesCapacity = initialStageCapacity * (sizeof(BatchStage)+sizeof(ClearStage))/2;
        _bytesCapacity = std::max(_bytesCapacity, (unsigned)sizeof(ClearStage));
        _bytesCapacity += initialStageCapacity * sizeof(Stage*);
        _start = (Stage*)AllocateMemory<char>(_manager._allocator, _bytesCapacity);
        _end = _start;
    }

    Composer::~Composer()
    {
        FreeMemory(_start);
    }

    void Composer::reset()
    {
        for (int i = -(int)_count; i < 0; i++)
        {
            ReleaseRef(_stages[i]->targets);
        }

        _end = _start;
        _count = 0;
    }

    void Composer::reserve(unsigned bytes)
    {
        if ((char*)_end + bytes > (char*)(_stages - _count - 1))
        {
            _bytesCapacity *= 2;
            Stage* start = (Stage*)AllocateMemory<char>(_manager._allocator, _bytesCapacity);
            Stage** stages = (Stage**)((char*)start + _bytesCapacity);
            memcpy(start, _start, (char*)_end - (char*)_start);

            ptrdiff_t offset = (char*)start - (char*)_start;
            for (int i = -(int)_count; i < 0; i++)
            {
                stages[i] = _stages[i];
                (char*&)stages[i] += offset;
            }

            FreeMemory(_start);
            _start = start;
            (char*&)_end += offset;
            _stages = stages;
        }
    }

    void Composer::saveToPipeline(Pipeline* pipeline) const
    {
    }

    PipelinePtr Composer::createPipeline() const
    {
        return nullptr;
    }

}
