#pragma once

#include "core/RefCounted.h"
#include "core/PodDeque.h"
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

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    protected:

        friend void             DestroyRefCounted(Display*);
        friend void             DestroyRefCounted(Texture*);
        friend void             DestroyRefCounted(TargetSet*);
        friend void             DestroyRefCounted(Pipeline*);

        struct DeadMeat
        {
            void*               object;
            DeleteFunc          deleteFunc;
            unsigned            frameNumber;
        };

        Error                   platformInit(const Config& config);
        void                    platformCleanup();

                                template<class T>
        void                    scheduleDeletion(T* obj, unsigned delay);

        enum {                  MaxWorklists = 8 };

        Config                  _config;

        BlockAllocator          _displayAllocator;
        BlockAllocator          _textureAllocator;
        BlockAllocator          _targetSetAllocator;
        PipelineManager         _pipelineManager;
        Keysmith<RenderPort>    _portSmith;
        PodDeque<DeadMeat>      _deadMeat;

        int8_t*                 _scratchMem      = 0;

        std::atomic<int8_t*>    _scratchAllocPtr = 0;
        int8_t*                 _scratchAllocEnd = 0;

        Worklist                _worklists[MaxWorklists];
        unsigned                _worklistCount   = 0;

        unsigned                _frameNumber     = 0;

        void*                   _platformDetails[8];

    public:

        PlatformDetails&        getPlatformDetails();
        PipelineManager&        getPipelineManager();
        int8_t*                 scratchAlloc(unsigned bytes);

    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    inline Renderer::Renderer()
    {
    }

    inline RenderPort* Renderer::getPort(const char* name) throw()
    {
        return _portSmith.issue(name);
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

    template<class T> void Renderer::scheduleDeletion(T* obj, unsigned delay)
    {
        assert(delay > 0);
        Renderer::DeadMeat& deadMeat = _deadMeat.addLast();
        deadMeat.object = obj;
        deadMeat.deleteFunc = (DeleteFunc)Delete<T>;
        deadMeat.frameNumber = _frameNumber + delay;
    }

    inline PipelineManager& Renderer::getPipelineManager()
    {
        return _pipelineManager;
    }

}

