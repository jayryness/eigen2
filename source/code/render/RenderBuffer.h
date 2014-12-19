#pragma once

#include "RenderStruct.h"
#include "core/RefCounted.h"
#include "core/Error.h"
#include "core/EnumBits.h"

namespace eigen
{

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderBuffer
    //
    // A one-dimensional GPU resource used for vertices, indices, shader constants, intermediate results, etc.
    // Configuration of RenderBuffers is tricky and requires complete knowledge of how the buffer is to be used by
    // the shader. For this reason it isn't recommended to configure them manually, but instead to get the configuration
    // from the shader. TODO
    //

    class RenderBuffer : public RefCounted<RenderBuffer>
    {
    public:

        enum class Arena        : uint8_t
        {
            GpuExclusive = 0,
            Cooperative,                // CPU rewritable
            ShaderVars                  // [requires Bindings == None]
        };

        enum class Bindings     : uint8_t
        {
            None                = 0,
            FunctionArgs        = 1<<0, // higher-order GPU programming
            Scratch             = 1<<1, // read/write from any shader
            Vertices            = 1<<2, // vertex shader stream-in
            Indices             = 1<<3, // input assembler stream-in
            RenderTarget        = 1<<4, // pixel shader output [probably requires Arena == GpuExclusive]
        };

        struct Config
        {
            Arena           arena           = Arena::GpuExclusive;
            Bindings        bindings        = Bindings::None;
            uint32_t        elementStride   = 0;
            uint32_t        elementCount    = 0;
        };

        struct Slice
        {
            uint32_t        elementStart    = 0;
            uint32_t        elementEnd      = 0;
        };

        Error               initialize(const Config& config);
        void                detach();   // Release GPU resources early

        const Config&       getConfig() const;

    protected:

                            RenderBuffer();
                           ~RenderBuffer();

        Error               platformInit(const Config& config);
        void                platformDetach();

        Config             _config;
    };

    typedef RefPtr<RenderBuffer> RenderBufferPtr;

    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    EIGEN_DEFINE_ENUM_BIT_OPS(RenderBuffer::Bindings)

    inline RenderBuffer::RenderBuffer()
    {
    }

    inline RenderBuffer::~RenderBuffer()
    {
    }

    inline const RenderBuffer::Config& RenderBuffer::getConfig() const
    {
        return _config;
    }

    inline void RenderBuffer::detach()
    {
        platformDetach();
    }
}
