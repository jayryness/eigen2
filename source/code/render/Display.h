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

    class Display         : public RefCounted<Display>
    {
    public:

        Error               bindToWindow(void* windowHandle);

        Texture*            getTarget() const;

    protected:
                            friend class DisplayManager;

                            Display() {}
                            ~Display();

        void                present();

        TexturePtr          _target;
        int                 _index = 0;
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
