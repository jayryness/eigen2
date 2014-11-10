#pragma once

#include <cstdint>

namespace eigen
{
    template<int N> class BitSet
    {
    public:

        enum {      Size = N };

                    BitSet();

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
        void        complement();
        void        clearExceptLsb();

                    template<typename T_FUNC>
        void        forEach(T_FUNC func);       // func(unsigned position, const BitSet& bit)

        void        getRange(unsigned& start, unsigned& end) const; // [start, end)
        unsigned    hash() const;

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

    template<int N> void BitSet<N>::clearExceptLsb()
    {
        for (unsigned i = 0; i < Parts; i++)
        {
            if (_parts[i] != 0)
            {
                _parts[i] &= -(int64_t)_parts[i];

                i++;
                for (; i < Parts; i++)
                {
                    _parts[i] = 0;
                }

                break;
            }
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
                start += LocateBit(x);
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
                end += LocateBit(x);
                return;
            }
        }

        assert(false);
    }

    template<int N> unsigned BitSet<N>::hash() const
    {
        // Murmur2-64
        enum : uint64_t { m = 0xc6a4a7935bd1e995 };
        enum { r = 47 };
        uint64_t h = Parts*8 + 761110;

        for (unsigned i = 0; i < Parts; i++)
        {
            uint64_t k = _parts[i] * m;
            k ^= k >> r;
            k *= m;
            h = ( h * m ) ^ k;
        }
        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return (unsigned)(sizeof(unsigned) < 8 ? (h >> 32) : h);
    }

    template<int N> template<typename T_FUNC> void BitSet<N>::forEach(T_FUNC func)
    {
        BitSet bit;

        for (unsigned i = 0; i < Parts; i++)
        {
            if (_parts[i] == 0)
                continue;

            do
            {
                bit._parts[i] = _parts[i] & -(int64_t)_parts[i];

                unsigned pos = i*64;
                pos += LocateBit(bit._parts[i]);

                do
                {
                    if (bit._parts[i] & _parts[i])
                    {
                        func(pos, bit);
                    }
                    pos++;
                    bit._parts[i] <<= 1;
                }   while (bit._parts[i] <= _parts[i]);
                i++;
            }   while (_parts[i] && i < Parts);

            return;
        }
    }

}
