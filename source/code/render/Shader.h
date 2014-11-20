#pragma once

#include "core/RefCounted.h"
#include "core/Key.h"

namespace eigen
{

    // TODO clean this up

    enum
    {
        MaxShaderVariants   = 128,
        MaxBufferAttributes = 64,
    };

    enum { MaxBufferAttributes = 64 };

    class ShaderVariantIdentifier : public Key<ShaderVariantIdentifier, MaxShaderVariants>
    {
    protected:

        ShaderVariantIdentifier() {}
    };

    class BufferAttribute : public Key<BufferAttribute, MaxBufferAttributes>
    {
    protected:

        BufferAttribute() {}
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Shader
    //

    class Shader : public RefCounted<Shader>
    {
    public:

        struct BufferInfo
        {
            BufferAttribute::Set            attributes;
            //DataSpec*                     layout;     // todo
            unsigned                        maxLength;
        };

        struct Style
        {
            ShaderVariantIdentifier::Set    variants;
            const char*                     name;
        };

        struct Config
        {
            BufferInfo*                     buffers;
            Style*                          styles;
            unsigned                        bufferCount;
            unsigned                        styleCount;
        };

        const Config&                       GetConfig() const;

    protected:
                                            Shader(const Config& info);
                                            ~Shader();

        Config                              _config;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

}
