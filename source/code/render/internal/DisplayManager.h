#pragma once

#include "../Display.h"

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

        void                presetAll();

    private:

                            enum { MaxDisplays = 16 };

        Display*            _displays[MaxDisplays];
    };


}
