#include "RenderPlan.h"
#include "Renderer.h"

namespace eigen
{
    inline Stage* NextStage(Stage* prev)
    {
        switch (prev->type)
        {
        case Stage::Type::Clear:    return (ClearStage*)prev + 1;
        case Stage::Type::Batch:    return (BatchStage*)prev + 1;
        case Stage::Type::Filter:   return (FilterStage*)prev + 1;
        }
        assert(false);
        return nullptr;
    }

    RenderPlan::RenderPlan()
    {
    }

    RenderPlan::~RenderPlan()
    {
        if (--Allocation::From(_start)->_metadataInt == 0)
        {
            for (Stage* stage = _start; stage < _end; stage = NextStage(stage))
            {
                ReleaseRef(stage->targets);
            }
            FreeMemory(_start);
        }
    }

    Error RenderPlan::initialize(Stage** stages, unsigned stageCount)
    {
        assert(_start == nullptr);     // already initialized

        _portSet.clear();

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
                _portSet |= ((BatchStage*)stages[i])->renderPort->getBit();
                bytes += sizeof(BatchStage);
                break;
            case Stage::Type::Filter:
                bytes += sizeof(FilterStage);
                break;
            default:
                EIGEN_RETURN_ERROR("Invalid stage type at location %d", (long)i);
            }
        }

        _start = (Stage*)AllocateMemory<char>(_manager->_allocator, bytes);
        Allocation::From(_start)->_metadataInt = 1; // sneaky refcount in allocation metadata
        _count = stageCount;

        _end = _start;
        for (unsigned i = 0; i < stageCount; i++)
        {
            AddRef(_end->targets);
            _end = NextStage(_end);
        }

        EIGEN_RETURN_OK();
    }

    Error RenderPlan::initialize(const RenderPlanner& planner)
    {
        assert(_start == nullptr);     // already initialized

        RenderPort::Set portSet;

        unsigned count = 0;
        for (Stage* stage = planner._start; stage < planner._end; stage = NextStage(stage))
        {
            if (stage->type == Stage::Type::Batch)
            {
                RenderPort* port = ((BatchStage*)stage)->renderPort;
                if (port == nullptr)
                {
                    EIGEN_RETURN_ERROR("Stage %d has null render port", (long)count);
                }
                portSet |= port->getBit();
            }
            count++;
        }

        _start = planner._start;
        _end = planner._end;
        _count = planner._count;
        _portSet = portSet;

        Allocation::From(_start)->_metadataInt++;   // sneaky refcount

        EIGEN_RETURN_OK();
    }

    void RenderPlanManager::initialize(Allocator* allocator, unsigned initialCapacity)
    {
        assert(_allocator == nullptr);  // already initialized
        _allocator = allocator;

        _planAllocator.initialize(allocator, sizeof(RenderPlan), initialCapacity);
    }

    RenderPlanner::RenderPlanner(Renderer& renderer, unsigned initialStageCapacity)
        : _manager(renderer.getPlanManager())
    {
        _bytesCapacity = initialStageCapacity * (sizeof(BatchStage)+sizeof(ClearStage))/2;
        _bytesCapacity = std::max(_bytesCapacity, (unsigned)sizeof(ClearStage));
        _start = (Stage*)AllocateMemory<char>(_manager._allocator, _bytesCapacity);
        Allocation::From(_start)->_metadataInt = 1; // sneaky refcount in allocation metadata
        _end = _start;
    }

    RenderPlanner::~RenderPlanner()
    {
        if (--Allocation::From(_start)->_metadataInt == 0)
        {
            for (Stage* stage = _start; stage < _end; stage = NextStage(stage))
            {
                ReleaseRef(stage->targets);
            }
            FreeMemory(_start);
        }
    }

    void RenderPlanner::reset()
    {
        _end = _start;
        _count = 0;
    }

    void RenderPlanner::reserve(unsigned bytes)
    {
        bool shared = Allocation::From(_start)->_metadataInt > 1;
        bool exhausted = (char*)_end + bytes > (char*)(_start + _bytesCapacity);
        if (shared | exhausted)
        {
            _bytesCapacity *= exhausted ? 2 : 1;
            Allocation::From(_start)->_metadataInt--;
            Stage* start = (Stage*)AllocateMemory<char>(_manager._allocator, _bytesCapacity);
            Allocation::From(start)->_metadataInt = 1; // sneaky refcount in allocation metadata
            memcpy(start, _start, (char*)_end - (char*)_start);
            FreeMemory(_start);

            _start = start;
            (char*&)_end += (char*)start - (char*)_start;
        }
    }

}
