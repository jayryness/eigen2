#pragma once

#include "memory.h"
#include "BitSet.h"
#include "hash.h"
#include "math.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Flag
    //
    // An bit flag with interned string identifier
    //

    template<class T, int MAX> class Flag
    {
    public:
        typedef BitSet<MAX> Set;

        enum                { Max = MAX };

        const char*         getName() const;
        unsigned            getPosition() const;
        const BitSet<MAX>&  getBit() const;

    protected:

                            Flag() {}

        const char*        _name;
        Set                _bit;
        unsigned           _position;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // FlagIssuer
    //
    // Manages flags and their strings
    //

    template<class T> class FlagIssuer
    {
    public:

                            FlagIssuer();
                            ~FlagIssuer();

        void                initialize(Allocator* allocator, unsigned initialStringsCapacity);

        unsigned            getCount() const;
        T*                  issue(const char* name);

    protected:

        struct Flag :       public T
        {
                            friend class FlagIssuer;
        };

        enum                { TableSize = StaticNextPow2<T::Max*2>::Result };

        unsigned            _hashes[TableSize];
        int16_t             _indices[TableSize];
        Flag                _inventory[T::Max];
        unsigned            _count              = 0;

        char*               _strings            = nullptr;
        unsigned            _stringsEnd         = 0;
        unsigned            _stringsCapacity    = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

    template<class T, int MAX> const char* Flag<T,MAX>::getName() const
    {
        return _name;
    }

    template<class T, int MAX> unsigned Flag<T,MAX>::getPosition() const
    {
        return _position;
    }

    template<class T, int MAX> const BitSet<MAX>& Flag<T,MAX>::getBit() const
    {
        return _bit;
    }

    template<class T> FlagIssuer<T>::FlagIssuer()
    {
        memset(_hashes, 0, sizeof(_hashes));
        memset(_indices, -1, sizeof(_indices));
    }

    template<class T> FlagIssuer<T>::~FlagIssuer()
    {
        FreeMemory(_strings);
    }

    template<class T> void FlagIssuer<T>::initialize(Allocator* allocator, unsigned initialStringsCapacity)
    {
        assert(_strings == nullptr);
        _strings = AllocateMemory<char>(allocator, initialStringsCapacity);
        _stringsEnd = 0;
        _stringsCapacity = initialStringsCapacity;
    }

    template<class T> unsigned FlagIssuer<T>::getCount() const
    {
        return _count;
    }

    template<class T> T* FlagIssuer<T>::issue(const char* name) throw()
    {
        unsigned nameLength = (unsigned)strlen(name);

        // Find the port

        unsigned hash = Hash32(name, nameLength);
        unsigned slot = hash & (TableSize-1);
        while (_hashes[slot] != 0)
        {
            if (_hashes[slot] == hash)
            {
                // Found it

                T* flag = _inventory + _indices[slot];
                return flag;
            }

            slot = (slot+1) & (TableSize-1);
        }

        // It's not in the table - can we add it?

        if (_count == T::Max)
        {
            assert(false);
            return nullptr;
        }

        // Reserve space in string table and copy name

        if (_stringsEnd + nameLength + 1 > _stringsCapacity)
        {
            _stringsCapacity = (_stringsEnd + nameLength + 1) * 2;
            Allocation* allocation = Allocation::From(_strings);
            char* strings = AllocateMemory<char>(allocation->_allocator, _stringsCapacity);
            memcpy(strings, _strings, _stringsEnd);
            FreeMemory(_strings);
            _strings = strings;
        }
        memcpy(_strings + _stringsEnd, name, nameLength + 1);

        // Populate the slot with the new flag

        _hashes[slot] = hash;
        _indices[slot] = (int16_t)_count;

        Flag* flag = _inventory + _count;
        flag->_name = _strings + _stringsEnd;
        flag->_bit.clear();
        flag->_bit.set(_count, true);
        flag->_position = _count;

        _stringsEnd += nameLength + 1;
        _count++;

        return flag;
    }

}
