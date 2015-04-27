#pragma once

#include "core/RefCounted.h"
#include "core/SoftBitFlag.h"

namespace eigen
{

    struct RenderStruct;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // EffectAspect
    //
    // Represents a specific configuration of the shader pipeline for a certain look or
    // technique. For example, a "depth only" aspect might be used to render shadowmaps.
    //

    enum {  MaxEffectAspects        = 64 };

    class EffectAspect :            public SoftBitFlag<EffectAspect, MaxEffectAspects>
    {
    protected:                      EffectAspect() {}
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // BufferSemantic
    //

    //enum {  MaxBufferSemantics      = 64 };

    //class BufferSemantic :          public Flag<BufferSemantic, MaxBufferSemantics>
    //{
    //protected:                      BufferSemantic() {}
    //};

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Effect
    //
    // A configuration of the entire programmable GPU pipeline, including all applicable
    // shader stages and render states.
    //

    class Effect :                  public RefCounted<Effect>
    {
    public:

        struct BufferInfo
        {
            const char*             semantic;
            RenderStruct*           layout;
            unsigned                maxLength;
        };

        struct Info
        {
            BufferInfo*             buffers;
            unsigned                bufferCount;
            EffectAspect::Set       aspects;
        };

        const Info&                 getInfo() const;

    protected:
                                    Effect(const Info& info);
                                    ~Effect();

        Info                        _info;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

}
