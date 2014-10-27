#pragma once

#include "Allocator.h"
#include <algorithm>

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // PodDeque
    //
    // Note: For POD types only. Constructor/destructor of contained type is not called.
    //

    template<typename T> class PodDeque
    {
    public:
                        PodDeque();
                       ~PodDeque();

        void            initialize(Allocator* allocator, unsigned initialCapacity);

                        template<typename T_INDEX>
        T&              at(T_INDEX index);

                        template<typename T_INDEX>
        const T&        at(T_INDEX index) const;

        T&              addLast();
        void            removeLast();

        T&              addFirst();
        void            removeFirst();

        unsigned        getCount();

    private:

        void            reserve();

        T*             _elements    = nullptr;
        unsigned       _count       = 0;
        unsigned       _start       = 0;
        unsigned       _mask        = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    template<typename T> PodDeque<T>::PodDeque()
    {
    }

    template<typename T> PodDeque<T>::~PodDeque()
    {
        FreeMemory(_elements);
    }

    template<typename T> void PodDeque<T>::initialize(Allocator* allocator, unsigned initialCapacity)
    {
        assert(_elements == nullptr);   // already initialized

        initialCapacity--;
        initialCapacity |= initialCapacity>>1;
        initialCapacity |= initialCapacity>>2;
        initialCapacity |= initialCapacity>>4;
        initialCapacity |= initialCapacity>>8;
        initialCapacity |= initialCapacity>>16;
        initialCapacity++;

        _elements = AllocateMemory<T>(allocator, initialCapacity);
        _mask = initialCapacity - 1;
    }

    template<typename T> template<typename T_INDEX> T& PodDeque<T>::at(T_INDEX index)
    {
        assert((unsigned)index < _count);
        index += _start;
        index &= _mask;
        return _elements[index];
    }

    template<typename T> template<typename T_INDEX> const T& PodDeque<T>::at(T_INDEX index) const
    {
        assert((unsigned)index < _count);
        index += _start;
        index &= _mask;
        return _elements[index];
    }

    template<typename T> T& PodDeque<T>::addLast()
    {
        reserve();
        unsigned index = _start + _count++;
        index &= _mask;
        return _elements[index];
    }

    template<typename T> void PodDeque<T>::removeLast()
    {
        assert(_count);
        _count--;
    }

    template<typename T> T& PodDeque<T>::addFirst()
    {
        reserve();
        _count++;
        _start--;
        _start &= _mask;
        return _elements[_start];
    }

    template<typename T> void PodDeque<T>::removeFirst()
    {
        assert(_count);
        _count--;
        _start++;
        _start &= _mask;
    }

    template<typename T> unsigned PodDeque<T>::getCount()
    {
        return _count;
    }

    template<typename T> void PodDeque<T>::reserve()
    {
        assert(_elements);      // not initialized

        if (_count == _mask)
        {
            unsigned capacity = _mask*2 + 1;
            T* elements = AllocateMemory<T>(Allocation::From(_elements)->_allocator, capacity);
            memcpy(elements, _elements+_start, std::min(_count, _mask-_start));
            memcpy(elements+_mask-_start, _elements, std::min(_start+_count,_mask)-_mask);
            FreeMemory(_elements);
            _elements = elements;
            _mask = capacity;
        }
    }

}
