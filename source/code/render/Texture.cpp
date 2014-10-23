#include "Texture.h"

namespace eigen
{

    Error Texture::initialize(const Texture::Config& config)
    {
        detach();
        _config = config;
        return platformInit(config);
    }

}
