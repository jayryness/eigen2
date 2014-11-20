#include "DisplayManager.h"

namespace eigen
{

    DisplayManager::DisplayManager()
    {
    }

    DisplayManager::~DisplayManager()
    {
    }

    void DisplayManager::initialize(Allocator* allocator)
    {
        _displays.initialize(allocator, 8);
        platformInit(allocator);
    }

    void DisplayManager::presentAll(unsigned frameNumber)
    {
        for (unsigned i = 0; i < _displays.getCount(); i++)
        {
            Display* display = _displays.at(i);
            display->present();     // TODO only present dirty displays
        }
    }

}
