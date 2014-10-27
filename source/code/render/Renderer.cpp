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
        _portManager.initialize(config.allocator, 2048);
        _pipelineManager.initialize(config.allocator, 8);
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

        _displayManager.cleanup();
        //_targetSetManager.cleanup();
        _textureManager.cleanup();
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

    PipelinePtr Renderer::createPipeline()
    {
        return _pipelineManager.create();
    }

    void DestroyRefCounted(Pipeline* pipeline)
    {
        Renderer& renderer = Renderer::From(pipeline->getManager());
        renderer.scheduleDeletion(pipeline, 1);
    }

    Worklist* Renderer::openWorklist(const Pipeline* pipeline)
    {
        if (_worklistCount == MaxWorklists)
        {
            return nullptr;
        }

        Worklist* worklist = new(_worklists + _worklistCount++) Worklist(this, pipeline);
        return worklist;
    }

    void Renderer::commenceWork()
    {
        // Ensure that no worklists were left open

        for (unsigned i = 0; i < _worklistCount; i++)
        {
            if (_worklists[i]._renderer)
            {
                // error TODO
                assert(false);      // must call Worklist::finish() on all open worklists before commencing work
                return;
            }
        }

        // TODO: sync to submission thread

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

        //_displayManager.destroyGarbage();
        //_textureManager.destroyGarbage();
        //_targetSetManager.destroyGarbage();

        // Perform delayed destruction on resources

        while (_deadMeat.getCount() && _deadMeat.at(0).frameNumber == _frameNumber)
        {
            DeadMeat& meat = _deadMeat.at(0);
            meat.deleteFunc(meat.object);
            _deadMeat.removeFirst();
        }

        _worklistCount = 0;
        _frameNumber++;
        // TODO: kick submission thread
    }
}
