#pragma once

#include "../Display.h"
#include "core/PodArray.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // DisplayManager
    //

    class DisplayManager
    {
    public:
                            DisplayManager();
                            ~DisplayManager();

        void                initialize(Allocator* allocator);

        Display*            createDisplay();
        void                unregisterDisplay(Display* display);

        void                presentAll(unsigned frameNumber);

    private:

        friend void         DestroyRefCounted(Display*);

        void                platformInit(Allocator* allocator);

        BlockAllocator      _blockAllocator;
        PodArray<Display*>  _displays;
    };


}
