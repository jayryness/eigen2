#pragma once

#include "core/RefCounted.h"
#include "core/Flag.h"

namespace eigen
{

    struct RenderStruct;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // ShaderAspect
    //
    // Represents a specific configuration of the shader pipeline for a certain look or
    // technique. It's common for a shader to have both "normal" and "depth only" aspects, the
    // latter being used to render shadowmaps.
    //

    enum {  MaxShaderAspects        = 64 };

    class ShaderAspect :            public Flag<ShaderAspect, MaxShaderAspects>
    {
    protected:                      ShaderAspect() {}
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
    // Shader
    //

    class Shader :                  public RefCounted<Shader>
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
            ShaderAspect::Set       aspects;
        };

        const Info&                 getInfo() const;

    protected:
                                    Shader(const Info& info);
                                    ~Shader();

        Info                        _info;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

}
