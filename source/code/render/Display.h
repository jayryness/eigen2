#pragma once

#include "Texture.h"

namespace eigen
{

    class Renderer;
    class Error;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Display
    //

    class Display         : public Managed<Display>
    {
    public:

        Error               bindToWindow(void* windowHandle);

        Texture*            getTarget() const;

        void                present();

    protected:

                            Display() {}
                            ~Display();

        TexturePtr         _target;
        Display*           _next    = nullptr;
    };

    typedef RefPtr<Display> DisplayPtr;

    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

    inline Display::~Display()
    {
    }

    inline Texture* Display::getTarget() const
    {
        return _target.ptr;
    }

}
