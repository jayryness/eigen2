#pragma once

#include <cassert>
#include <cstdint>

namespace eigen
{
    class Shader;
    class RenderData;       // collection of buffers conforming to shader specifications
    class ParameterBlock;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderBatch
    //
    // A unit of rendering work corresponding to one invocation of the shader pipeline (a.k.a. "draw call").
    //
    // Consists of:
    //   - Shader, the program executed by the GPU
    //   - RenderData, a collection of buffers conforming to shader specifications (a.k.a. "geometry")
    //   - ParameterBlocks[], shader input variables
    //
    // There are two ways to create a RenderBatch:
    //   1. Renderer::createBatch, makes a variable-lifetime batch, which holds references to its resources
    //   2. Worklist::transientBatch, makes a single-frame disposable batch in scratch memory
    //
    // Note that the actual memory footprint of a RenderBatch varies with the number and size of associated parameter
    // blocks dictated by the shader.
    //

    class RenderBatch
    {
    public:
        // Renderer::createBatch
        // Worklist::transientBatch

        // TODO shader has complete batch prototype so new batches can be initialized with memcpy

        Shader*             getShader() const;
        RenderData*         getData() const;
        unsigned            getParameterBlockCount() const;
        ParameterBlock*     getParameterBlock(int i) const;

    protected:
                            RenderBatch();
                            ~RenderBatch();

        uint8_t*            getParameterBlockOffsets() const;
        ParameterBlock*     firstParameterBlock() const;

        Shader*             _shader;
        RenderData*         _data;
        unsigned            _bytes;
        unsigned            _parameterBlockCount;

    };

    inline uint8_t* RenderBatch::getParameterBlockOffsets() const
    {
        return (uint8_t*)(this+1);
    }

    inline ParameterBlock* RenderBatch::firstParameterBlock() const
    {
        return (ParameterBlock*)(getParameterBlockOffsets() + _parameterBlockCount);
    }

    inline ParameterBlock* RenderBatch::getParameterBlock(int i) const
    {
        assert((unsigned)i < _parameterBlockCount);
        uint8_t* offsets = getParameterBlockOffsets();
        unsigned byteOffset = offsets[i] * 16;
        auto parameterBlock = (ParameterBlock*)((uint8_t*)firstParameterBlock() + byteOffset);
        return parameterBlock;
    }

}
