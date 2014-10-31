#include "Renderer.h"

namespace eigen
{

    Error Renderer::initialize(const Config& config)
    {
        assert(_frameNumber == 0);
        _frameNumber = 1;
        _config = config;

        _scratchMem = AllocateMemory<int8_t>(config.allocator, config.scratchSize);
        _scratchAllocPtr = _scratchMem;
        _scratchAllocEnd = _scratchMem + config.scratchSize/2;

        _deadMeat.initialize(config.allocator, 64);
        _portSmith.initialize(config.allocator, 2048);
        _planManager.initialize(config.allocator, 8);
        return platformInit(config);    // see e.g. RendererDx11.cpp
    }

    void Renderer::cleanup()
    {
        assert(_frameNumber > 0);
        // todo - stop the world

        while (_deadMeat.getCount())
        {
            DeadMeat& meat = _deadMeat.at(0);
            meat.deleteFunc(meat.object);
            _deadMeat.removeFirst();
        }

        FreeMemory(_scratchMem);
        platformCleanup();
        _frameNumber = 0;
    }

    Renderer::~Renderer()
    {
        if (_frameNumber > 0)
        {
            cleanup();
        }
    }

    RenderPlanPtr Renderer::createPlan()
    {
        return _planManager.create();
    }

    void DestroyRefCounted(RenderPlan* plan)
    {
        Renderer& renderer = *StructFromMember(&Renderer::_planManager, plan->getManager());
        renderer.scheduleDeletion(plan, 1);
    }

    Worklist* Renderer::openWorklist(RenderPlan* plan)
    {
        bool exhausted = _worklistEnd == _worklistEndVacant;
        bool validated = Ok(plan->validate());
        if (exhausted || !validated)
        {
            return nullptr;
        }

        Worklist* worklist = new(_worklists + _worklistEnd) Worklist(this, plan);
        _worklistEnd = (_worklistEnd + 1) % MaxWorklists;
        return worklist;
    }

    void Renderer::commenceWork()
    {
        // Ensure that no worklists were left open

        Worklist* head = nullptr;
        Worklist** tail = &head;
        for (unsigned i = _worklistStart; i != _worklistEnd; i = (i+1) % MaxWorklists)
        {
            if (_worklists[i]._renderer)
            {
                // error TODO
                assert(false);      // must call Worklist::finish() on all open worklists before commencing work
                return;
            }

            *tail = _worklists + i;
            tail = &(*tail)->_next;
        }
        *tail = nullptr;

        // TODO: sync to submission thread
        // retire completed worklists
        // attach worklists
        // kick submission thread

        if (_scratchAllocPtr > _scratchAllocEnd)
        {
            // TODO grow scratch buffer or spew error?
            // note: must defer deallocation of current one until next frame if reallocting
        }

        // Toggle double-buffered frame alloc segment of scratch mem

        if (_scratchAllocEnd == _scratchMem + _config.scratchSize/2)
        {
            _scratchAllocPtr  = _scratchAllocEnd;
            _scratchAllocEnd += _config.scratchSize/2;
        }
        else
        {
            _scratchAllocPtr  = _scratchMem;
            _scratchAllocEnd -= _config.scratchSize/2;
        }

        // Perform delayed destruction on resources

        while (_deadMeat.getCount() && _deadMeat.at(0).frameNumber == _frameNumber)
        {
            DeadMeat& meat = _deadMeat.at(0);
            meat.deleteFunc(meat.object);
            _deadMeat.removeFirst();
        }

        _worklistEndVacant = _worklistStart;
        _worklistStart = _worklistEnd;
        _frameNumber++;
        // TODO: kick submission thread
    }
}
