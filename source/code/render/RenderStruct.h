#pragma once

#include "Format.h"
#include "core/types.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // RenderStruct
    //

    struct RenderStruct
    {
        struct Member
        {
            enum class Type     : unsigned
            {
                Compound        = 0x0ff,

                Half            = 0x121,
                Half2           = 0x122,
                Half3           = 0x123,
                Half4           = 0x124,
                Float           = 0x141,
                Float2          = 0x142,
                Float3          = 0x143,
                Float4          = 0x144,
                Double          = 0x181,
                Double2         = 0x182,
                Double3         = 0x183,
                Double4         = 0x184,

                Byte            = 0x211,
                Byte2           = 0x212,
                Byte3           = 0x213,
                Byte4           = 0x214,
                Short           = 0x221,
                Short2          = 0x222,
                Short3          = 0x223,
                Short4          = 0x224,
                Int             = 0x241,
                Int2            = 0x242,
                Int3            = 0x243,
                Int4            = 0x244,
            };

            const char*         name;
            RenderStruct*       compound;       // null unless type == Compound
            union
            {
                Type            type;
                Format          format;
            };
            unsigned            fixedLength;    // for fixed arrays
            unsigned            offset;         // in bytes from start of struct
        };

        const char*             structName;
        Member*                 members;
        unsigned                memberCount;
    };

    template<typename T, int N> bool WriteMember(uint8_t* data, RenderStruct::Member::Type type, const FixedN<T,N>& vector);
    template<typename T, int N> bool WriteMember(uint8_t* data, RenderStruct::Member::Type type, const FixedN<T,N>* array, int count);

    template<typename T, int N> bool ReadMember(const uint8_t* data, RenderStruct::Member::Type type, FixedN<T,N>& vector);
    template<typename T, int N> bool ReadMember(const uint8_t* data, RenderStruct::Member::Type type, FixedN<T,N>* array, int count);
}
