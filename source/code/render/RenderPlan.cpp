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

    inline unsigned SizeFrom(Stage::Type stageType)
    {
        return (unsigned)NextStage(nullptr);
    }

    inline const char* AsString(Stage::Type stageType)
    {
        switch (stageType)
        {
        case Stage::Type::Clear:    return "Clear";
        case Stage::Type::Batch:    return "Batch";
        case Stage::Type::Filter:   return "Filter";
        default:                    return "Unknown";
        }
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

    Error RenderPlan::addStages(Stage** stages, unsigned stageCount)
    {
        Error error = validate();
        if (!Ok(error))
        {
            EIGEN_RETURN_ERROR("RenderPlan was invalid before addStages. Reason: \"%s\"", error.getText());
        }

        RenderPort::Set portSet;
        unsigned bytes = 0;
        for (unsigned i = 0; i < stageCount; i++)
        {
            unsigned size = SizeFrom(stages[i]->type);
            if (size == 0)
            {
                EIGEN_RETURN_ERROR("Invalid stage type at location %d", (long)i);
            }
            if (stages[i]->targets == nullptr)
            {
                EIGEN_RETURN_ERROR("Stage %d has invalid targets", (long)i);
            }
            if (stages[i]->type == Stage::Type::Batch)
            {
                if (((BatchStage*) stages[i])->renderPort == nullptr)
                {
                    EIGEN_RETURN_ERROR("Stage %d has null render port", (long) i);
                }
                portSet |= ((BatchStage*) stages[i])->renderPort->getBit();
            }

            bytes += size;
        }

        reserve((char*)_end - (char*)_start + bytes);

        for (unsigned i = 0; i < stageCount; i++)
        {
            Stage* next = NextStage(_end);
            memcpy(_end, stages[i], (char*)next - (char*)_end);
            AddRef(_end->targets);
            _end = next;
        }

        _validated = _end;
        _portSet = portSet;
        _count += stageCount;

        EIGEN_RETURN_OK();
    }

    void RenderPlan::reset()
    {
        _portSet.clear();
        _end = _validated = _start;
        _count = 0;
    }

    void RenderPlan::reserve(uintptr_t bytes)
    {
        bool shared = _start && Allocation::From(_start)->_metadataInt > 1;
        bool exhausted = (char*)_end + bytes > (char*)(_start + _bytesCapacity);
        if (shared | exhausted)
        {
            _bytesCapacity = std::max(_bytesCapacity, (unsigned)sizeof(ClearStage));
            _bytesCapacity *= exhausted ? 2 : 1;

            if (_start)
            {
                Allocation::From(_start)->_metadataInt--;
            }
            Stage* start = (Stage*)AllocateMemory<char>(_manager->_allocator, _bytesCapacity);
            Allocation::From(start)->_metadataInt = 1; // sneaky refcount in allocation metadata
            memcpy(start, _start, (char*)_end - (char*)_start);
            FreeMemory(_start);

            ptrdiff_t offset = (char*)start - (char*)_start;
            (char*&)_end += offset;
            (char*&)_validated += offset;
            
            _start = start;
        }
    }

    Error RenderPlan::validate()
    {
        while (_validated < _end)
        {
            if (_validated->targets == nullptr)
            {
                EIGEN_RETURN_ERROR("%sStage has invalid targets", AsString(_validated->type));
            }
            if (_validated->type == Stage::Type::Batch)
            {
                RenderPort* port = ((BatchStage*)_validated)->renderPort;
                if (port == nullptr)
                {
                    EIGEN_RETURN_ERROR("BatchStage has null render port", nullptr);
                }
                _portSet |= port->getBit();
            }

            _validated = NextStage(_validated);
        }

        EIGEN_RETURN_OK();
    }

    void RenderPlanManager::initialize(Allocator* allocator, unsigned initialCapacity)
    {
        assert(_allocator == nullptr);  // already initialized
        _allocator = allocator;

        _planAllocator.initialize(allocator, sizeof(RenderPlan), initialCapacity);
    }

}
