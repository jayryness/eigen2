#include "Error.h"
#include <cstdio>

namespace eigen
{
    ErrorMsg::ErrorMsg(const char* fmt, long arg)
    {
        _snprintf_s(text, sizeof(text), fmt, arg);
    }

    ErrorMsg::ErrorMsg(const char* fmt, double arg)
    {
        _snprintf_s(text, sizeof(text), fmt, arg);
    }

    ErrorMsg::ErrorMsg(const char* fmt, const char* arg)
    {
        _snprintf_s(text, sizeof(text), fmt, arg);
    }

}
