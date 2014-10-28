#pragma once

#include "memory.h"
#include "BitSet.h"
#include "hash.h"
#include "math.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Key
    //
    // An interned string identifier with metadata for enumeration and set membership
    //

    template<class T, int MAX> class Key
    {
    public:
        typedef BitSet<MAX> Set;

        enum              { Max = MAX };

        const char*         getName() const;
        unsigned            getIndex() const;
        const BitSet<MAX>&  getBit() const;

    protected:

                            Key() {}

        const char*        _name;
        Set                _bit;
        unsigned           _index;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Keysmith
    //
    // Issues keys
    //

    template<class T> class Keysmith
    {
    public:

                            Keysmith();
                           ~Keysmith();

        void                initialize(Allocator* allocator, unsigned initialStringsCapacity);

        unsigned            getCount() const;
        T*                  issue(const char* name);

    protected:

        struct Key        : public T
        {
                            friend class Keysmith;
        };

        enum {              TableSize = StaticNextPow2<T::Max*2>::Result };

        unsigned           _hashes      [TableSize];
        int16_t            _indices     [TableSize];
        Key                _keys        [T::Max];
        unsigned           _keyCount            = 0;

        char*              _strings             = nullptr;
        unsigned           _stringsEnd          = 0;
        unsigned           _stringsCapacity     = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

    template<class T, int MAX> const char* Key<T,MAX>::getName() const
    {
        return _name;
    }

    template<class T, int MAX> unsigned Key<T,MAX>::getIndex() const
    {
        return _index;
    }

    template<class T, int MAX> const BitSet<MAX>& Key<T,MAX>::getBit() const
    {
        return _bit;
    }

    template<class T> Keysmith<T>::Keysmith()
    {
        memset(_hashes, 0, sizeof(_keys));
        memset(_indices, -1, sizeof(_indices));
    }

    template<class T> Keysmith<T>::~Keysmith()
    {
        FreeMemory(_strings);
    }

    template<class T> void Keysmith<T>::initialize(Allocator* allocator, unsigned initialStringsCapacity)
    {
        assert(_strings == nullptr);
        _strings = AllocateMemory<char>(allocator, initialStringsCapacity);
        _stringsEnd = 0;
        _stringsCapacity = initialStringsCapacity;
    }

    template<class T> unsigned Keysmith<T>::getCount() const
    {
        return _keyCount;
    }

    template<class T> T* Keysmith<T>::issue(const char* name) throw()
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

                T* key = _keys + _indices[slot];
                return key;
            }

            slot = (slot+1) & (TableSize-1);
        }

        // It's not in the table - can we add it?

        if (_keyCount == T::Max)
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

        // Populate the slot with the new key

        _hashes[slot] = hash;
        _indices[slot] = (int16_t)_keyCount;

        Key* key = _keys + _keyCount;
        key->_name = _strings + _stringsEnd;
        key->_bit.clear();
        key->_bit.set(_keyCount, true);
        key->_index = _keyCount;

        _stringsEnd += nameLength + 1;
        _keyCount++;

        return key;
    }

}
