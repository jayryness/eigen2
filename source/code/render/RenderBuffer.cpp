#include "RenderBuffer.h"

namespace eigen
{

    Error RenderBuffer::initialize(const RenderBuffer::Config& config)
    {
        detach();
        _config = config;
        return platformInit(config);
    }

}
