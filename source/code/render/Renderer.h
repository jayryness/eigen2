#pragma once

#include "core/RefCounted.h"
#include "core/Pool.h"
#include "core/Allocator.h"
#include "core/Error.h"
#include "Worklist.h"
#include "Pipeline.h"
#include "Display.h"
#include "Texture.h"
#include "TargetSet.h"
#include "RenderPort.h"

namespace eigen
{

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Renderer
    {
    public:
                                struct PlatformConfig;
                                struct PlatformDetails;
        struct Config
        {
            Allocator*          allocator       = Mallocator::Get();
            bool                debugEnabled    = false;
            unsigned            scratchSize     = 16*1024*1024;
            PlatformConfig*     platformConfig  = nullptr;
        };

                                Renderer();
                               ~Renderer();

        Error                   initialize(const Config& config);
        void                    cleanup();          // Optional, handled by dtor

        Worklist*               openWorklist(const Pipeline* pipeline);

        void                    commenceWork();

        RenderPort*             getPort(const char* name);
        unsigned                getFrameNumber() const;

        DisplayPtr              createDisplay();
        TexturePtr              createTexture();
        TargetSetPtr            createTargetSet();
        PipelinePtr             createPipeline();

    protected:

        Error                   platformInit(const Config& config);
        void                    platformCleanup();

        enum {                  MaxWorklists = 8 };

        Config                 _config;

        Manager<Display>       _displayManager;
        Manager<Texture>       _textureManager;
        Manager<TargetSet>     _targetSetManager;
        PipelineManager        _pipelineManager;
        Keysmith<RenderPort>   _portManager;

        int8_t*                _scratchMem      = 0;

        std::atomic<int8_t*>   _scratchAllocPtr = 0;
        int8_t*                _scratchAllocEnd = 0;

        Worklist               _worklists[MaxWorklists];
        unsigned               _worklistCount   = 0;

        unsigned               _frameNumber     = 0;

        void*                  _platformDetails[8];

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    public:

        PlatformDetails&        getPlatformDetails();
        int8_t*                 scratchAlloc(unsigned bytes);

        static Renderer&        From(const Manager<Display>* manager);
        static Renderer&        From(const Manager<Texture>* manager);
        static Renderer&        From(const Manager<TargetSet>* manager);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    inline Renderer::Renderer()
    {
    }

    inline RenderPort* Renderer::getPort(const char* name) throw()
    {
        return _portManager.issue(name);
    }

    inline unsigned Renderer::getFrameNumber() const
    {
        return _frameNumber;
    }

    inline int8_t* Renderer::scratchAlloc(unsigned bytes)
    {
        int8_t* p = _scratchAllocPtr.fetch_add((bytes + 15) & ~15, std::memory_order_relaxed);  // p gets old value
        if (p + bytes > _scratchAllocEnd)
        {
            return nullptr;
        }
        return p;
    }

    inline Renderer& Renderer::From(const Manager<Display>* manager)
    {
        assert(manager);
        Renderer* renderer = (Renderer*)((uint8_t*)manager - offsetof(Renderer, _displayManager));
        return *renderer;
    }

    inline Renderer& Renderer::From(const Manager<Texture>* manager)
    {
        assert(manager);
        Renderer* renderer = (Renderer*)((uint8_t*)manager - offsetof(Renderer, _textureManager));
        return *renderer;
    }

    inline Renderer& Renderer::From(const Manager<TargetSet>* manager)
    {
        assert(manager);
        Renderer* renderer = (Renderer*)((uint8_t*)manager - offsetof(Renderer, _targetSetManager));
        return *renderer;
    }

}

