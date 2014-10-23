#pragma once

#include "Texture.h"

namespace eigen
{

    class Renderer;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // TargetSet
    //

    class TargetSet       : public Managed<TargetSet>
    {
    public:

        enum
        {
            MaxTextures = 8,
            MaxBuffers  = 8
        };

        struct Config
        {
                                    Config();
            Texture*                textures[MaxTextures];
            Texture::Slice          slices  [MaxTextures];
            //Buffer*               buffers [MaxBuffers];
            //Buffer::Range         ranges  [MaxBuffers];
            Texture*                zbuffer                     = 0;
            Texture::Slice          zbufferSlice;
        };

        Error               initialize(const Config& config);
        void                detach();   // Release GPU resources early

        const Config&       getConfig() const;
        unsigned            getTextureCount() const;

    protected:

                            TargetSet();
                           ~TargetSet();

        Error               platformInit(const Config& config);
        void                platformDetach();

        Config             _config;
        unsigned           _textureCount = 0;

    };

    typedef RefPtr<TargetSet> TargetSetPtr;

    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    inline TargetSet::Config::Config()
    {
        memset(textures, 0, sizeof(textures));
    }

    inline TargetSet::TargetSet()
    {
    }

    inline TargetSet::~TargetSet()
    {
        for (unsigned i=0; i<_textureCount; i++)
        {
            ReleaseRef(_config.textures[i]);
        }
        ReleaseRef(_config.zbuffer);
    }

    inline const TargetSet::Config& TargetSet::getConfig() const
    {
        return _config;
    }

    inline void TargetSet::detach()
    {
        platformDetach();
    }
}
