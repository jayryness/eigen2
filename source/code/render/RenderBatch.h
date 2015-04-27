#pragma once

#include <cassert>
#include <cstdint>

namespace eigen
{
    class Effect;
    class RenderData;       // collection of buffers conforming to Effect specifications
    class ParameterBlock;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderBatch
    //
    // A unit of rendering work corresponding to one invocation of the GPU pipeline (a.k.a. "draw call").
    //
    // Consists of:
    //   - Effect, the shader programs and related state for the GPU
    //   - RenderData, a collection of buffers conforming to effect specifications (a.k.a. "geometry")
    //   - ParameterBlocks[], effect input variables
    //
    // There are two ways to create a RenderBatch:
    //   1. Renderer::createBatch, makes a variable-lifetime batch, which holds references to its resources
    //   2. Worklist::transientBatch, makes a single-frame disposable batch in scratch memory
    //
    // Note that the actual memory footprint of a RenderBatch varies with the number and size of associated parameter
    // blocks dictated by the Effect.
    //

    class RenderBatch
    {
    public:
        // Renderer::createBatch
        // Worklist::transientBatch

        // TODO Effect has complete batch prototype so new batches can be initialized with memcpy

        Effect*             getEffect() const;
        RenderData*         getData() const;
        unsigned            getParameterBlockCount() const;
        ParameterBlock*     getParameterBlock(int i) const;

    protected:
                            RenderBatch();
                            ~RenderBatch();

        uint8_t*            getParameterBlockOffsets() const;
        ParameterBlock*     firstParameterBlock() const;

        Effect*             _effect;                // TODO handles instead of pointers for these (from e.g. Effect::getHandle()) (for compactness - once committed into the pipe, handle can resolve to uint16 index)
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
