#pragma once

#include <cstdint>

namespace eigen
{
    template<int N> class BitSet
    {
    public:

        enum {      Size = N };

                    BitSet();

        void        complement();

        void        operator&=(const BitSet& rhs);
        void        operator|=(const BitSet& rhs);
        void        operator^=(const BitSet& rhs);

        void        operator<<=(unsigned shift);
        void        operator>>=(unsigned shift);

        bool        operator==(const BitSet& rhs) const;
        bool        isEmpty() const;
        bool        isSubsetOf(const BitSet& set) const;
        bool        intersects(const BitSet& set) const;

        void        clear();
        void        set(unsigned position, bool value);

        void        getRange(unsigned& start, unsigned& end) const; // [start, end)

    private:

        enum
        {
            PartSize    = 64,
            Parts       = (N + PartSize - 1) / PartSize,
        };

        uint64_t   _parts[Parts];
    };

    template<int N> BitSet<N>::BitSet()
    {
        clear();
    }

    template<int N> void BitSet<N>::complement()
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            _parts[i] = ~_parts[i];
        }
    }

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

    template<int N> void BitSet<N>::operator<<=(unsigned shift)
    {
        for (unsigned i = Parts-1; i > 0; i--)
        {
            _parts[i] |= _parts[i-1] >> (64 - shift);
            _parts[i] <<= shift;
        }
        _parts[0] <<= shift;
    }

    template<int N> void BitSet<N>::operator>>=(unsigned shift)
    {
        for (unsigned i = 0; i < Parts-1; i++)
        {
            _parts[i] >>= shift;
            _parts[i] |= _parts[i+1] << (64 - shift);
        }
        _parts[Parts-1] >>= shift;
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

    template<int N> bool BitSet<N>::isEmpty() const
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            if (_parts[i] != 0)
            {
                return false;
            }
        }
        return true;
    }

    template<int N> void BitSet<N>::getRange(unsigned& start, unsigned& end) const
    {
        unsigned s = 0;
        for (; s < Parts; s++)
        {
            if (_parts[s] != 0)
            {
                uint64_t x = _parts[s];
                x &= -(int64_t)x;
                start = s*64;
                start += ((x & 0x00000000ffffffff) == 0) * 32;
                start += ((x & 0x0000ffff0000ffff) == 0) * 16;
                start += ((x & 0x00ff00ff00ff00ff) == 0) * 8;
                start += ((x & 0x0f0f0f0f0f0f0f0f) == 0) * 4;
                start += ((x & 0x3333333333333333) == 0) * 2;
                start += ((x & 0x5555555555555555) == 0) * 1;
                break;
            }
        }

        if (s == Parts)
        {
            start = end = 0;
            return;
        }

        for (unsigned e = Parts; e-- > 0;)
        {
            if (_parts[e] != 0)
            {
                uint64_t x = _parts[e];
                x |= x >> 1;
                x |= x >> 2;
                x |= x >> 4;
                x |= x >> 8;
                x |= x >> 16;
                x |= x >> 32;
                x >>= 1;
                x++;
                end = e*64 + 1;
                end += ((x & 0x00000000ffffffff) == 0) * 32;
                end += ((x & 0x0000ffff0000ffff) == 0) * 16;
                end += ((x & 0x00ff00ff00ff00ff) == 0) * 8;
                end += ((x & 0x0f0f0f0f0f0f0f0f) == 0) * 4;
                end += ((x & 0x3333333333333333) == 0) * 2;
                end += ((x & 0x5555555555555555) == 0) * 1;
                return;
            }
        }

        assert(false);
    }

}
