#include "TargetSet.h"

namespace eigen
{

    Error TargetSet::initialize(const TargetSet::Config& config)
    {
        detach();

        for (_textureCount = 0; _textureCount < MaxTextures && config.textures[_textureCount]; _textureCount++)
        {
            if (config.textures[_textureCount]->getConfig().usage != Texture::Usage::RenderTarget)
            {
                EIGEN_RETURN_ERROR("TargetSet requires texture usage 'RenderTarget' (see Texture::Config)", 0L);
            }
        }

        _config = config;
        return platformInit(config);
    }

}
