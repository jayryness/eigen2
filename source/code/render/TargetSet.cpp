#include "TargetSet.h"

namespace eigen
{

    Error TargetSet::initialize(const TargetSet::Config& config_)
    {
        Config config = config_;

        detach();

        for (_textureCount = 0; _textureCount < MaxTextures && config.textures[_textureCount]; _textureCount++)
        {
            if (config.textures[_textureCount]->getConfig().usage != Texture::Usage::RenderTarget)
            {
                EIGEN_RETURN_ERROR("TargetSet requires texture usage 'RenderTarget' (see Texture::Config)", 0L);
            }

            // fixup default slice

            if (config.slices[_textureCount].arrayEnd == 0)
            {
                config.slices[_textureCount].arrayEnd = config.textures[_textureCount]->getConfig().arrayLength;
            }
        }

        Error err = platformInit(config);
        if (ok(err))
        {
            _config = config;

            for (unsigned i = 0; i < _textureCount; i++)
            {
                AddRef(_config.textures[i]);
            }

            AddRef(_config.zbuffer);
        }
        return err;
    }

}
