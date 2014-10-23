#pragma once

#include <cstdint>

namespace eigen
{
    template<int N> class BitSet
    {
    public:

        enum {      Size = N };

        bool        operator==(const BitSet& rhs) const;

        void        operator&=(const BitSet& rhs);
        void        operator|=(const BitSet& rhs);
        void        operator^=(const BitSet& rhs);

        bool        isSubsetOf(const BitSet& set) const;
        bool        intersects(const BitSet& set) const;

        void        clear();
        void        set(unsigned position, bool value);

    private:

        enum
        {
            PartSize    = 64,
            Parts       = (N + PartSize - 1) / PartSize,
        };

        uint64_t   _parts[Parts];
    };

    template<int N> bool BitSet<N>::operator==(const BitSet& rhs) const
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            if (_parts[i] != rhs._parts[i])
            {
                return false;
            }
        }
        return true;
    }

    template<int N> void BitSet<N>::operator&=(const BitSet& rhs)
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            _parts[i] &= rhs._parts[i];
        }
    }

    template<int N> void BitSet<N>::operator|=(const BitSet& rhs)
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            _parts[i] |= rhs._parts[i];
        }
    }

    template<int N> void BitSet<N>::operator^=(const BitSet& rhs)
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            _parts[i] ^= rhs._parts[i];
        }
    }

    template<int N> bool BitSet<N>::isSubsetOf(const BitSet& other) const
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            if ((_parts[i] & other._parts[i]) != _parts[i])
            {
                return false;
            }
        }
        return true;
    }

    template<int N> bool BitSet<N>::intersects(const BitSet& other) const
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            if ((_parts[i] & other._parts[i]) != 0)
            {
                return true;
            }
        }
        return false;
    }

    template<int N> void BitSet<N>::clear()
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            _parts[i] = 0;
        }
    }

    template<int N> void BitSet<N>::set(unsigned position, bool value)
    {
        unsigned i = position / PartSize;
        position -= i*PartSize;
        if (value)
        {
            _parts[i] |= 1LL << position;
        }
        else
        {
            _parts[i] &= ~(1LL << position);
        }
    }

}
