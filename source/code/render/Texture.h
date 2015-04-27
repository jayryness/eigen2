#pragma once

#include "Format.h"
#include "core/RefCounted.h"
#include "core/Error.h"
#include "core/BitMaskOps.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Texture
    //

    class Texture                   : public RefCounted<Texture>
    {
    public:

        enum class Flags            : uint8_t
        {
            None                    = 0,
            CubeMap
        };

        enum class Usage            : uint8_t
        {
            Static                  = 0,
            Dynamic,
            RenderTarget
        };

        enum class Multisampling    : uint8_t
        {
            None                    = 0,
            X2,
            X4,
            X8,
        };

        struct Config
        {
            Format          format          = Format::Unspecified;
            Multisampling   multisampling   = Multisampling::None;
            Usage           usage           = Usage::Static;
            Flags           flags           = Flags::None;
            uint16_t        reserved        = 0;
            uint16_t        lastMip         = 0;
            uint16_t        width           = 0;
            uint16_t        height          = 0;
            uint16_t        depth           = 0;
            uint16_t        arrayLength     = 0;
        };

        struct Slice
        {
            uint16_t        mip             = 0;
            union           {
            uint16_t        arrayStart      = 0;
            uint16_t        depthStart;
                            };
            union           {
            uint16_t        arrayEnd        = 0;
            uint16_t        depthEnd;
                            };
        };

        Error               initialize(const Config& config);
        void                detach();   // Release GPU resources early

        const Config&       getConfig() const;

    protected:

                            Texture();
                           ~Texture();

        Error               platformInit(const Config& config);
        void                platformDetach();

        Config             _config;
    };

    typedef RefPtr<Texture> TexturePtr;

    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    EIGEN_DEFINE_BIT_MASK_OPS(Texture::Flags)

    inline Texture::Texture()
    {
    }

    inline Texture::~Texture()
    {
    }

    inline const Texture::Config& Texture::getConfig() const
    {
        return _config;
    }

    inline void Texture::detach()
    {
        platformDetach();
    }
}
