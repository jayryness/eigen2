#pragma once

#define EIGEN_RETURN_ERROR(fmt, arg)                \
    static eigen::ErrorMsg error##__LINE(fmt, arg); \
    return Error(&error##__LINE);

#define EIGEN_RETURN_OK() return Error()

namespace eigen
{

    struct ErrorMsg;

    class Error
    {
    public:
        Error(const ErrorMsg* msg = nullptr);

        friend bool Ok(const Error& error);
        friend bool Failed(const Error& error);

        const char* getText() const;

    protected:

        const ErrorMsg* _msg;
    };

    struct ErrorMsg
    {
        ErrorMsg(const char* fmt, long arg);
        ErrorMsg(const char* fmt, double arg);
        ErrorMsg(const char* fmt, const char* arg);

        char text[256];
    };

    inline Error::Error(const ErrorMsg* msg)
        : _msg(msg)
    {
    }

    inline bool Ok(const Error& error)
    {
        return error._msg == nullptr;
    }

    inline bool Failed(const Error& error)
    {
        return error._msg != nullptr;
    }

    inline const char* Error::getText() const
    {
        return _msg ? _msg->text : "";
    }
}
