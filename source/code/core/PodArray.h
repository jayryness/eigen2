#pragma once

#include "Allocator.h"

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // PodArray
    //
    // Note: For POD types only. Constructor/destructor of contained type is not called.
    //

    template<typename T> class PodArray
    {
    public:

                        PodArray(Allocator* allocator, unsigned initialCapacity);
                       ~PodArray();

                        template<typename T_INDEX>
        T&              at(T_INDEX);

                        template<typename T_INDEX>
        const T&        at(T_INDEX) const;

                        template<typename T_INDEX>
        void            remove(T_INDEX);    // swaps last into this slot

        T&              addLast();
        void            removeLast();

        void            setCount(unsigned count);
        unsigned        getCount();

        void            reserve(unsigned capacity, bool exact=true);

    private:

        T*             _elements    = nullptr;
        unsigned       _count       = 0;
        unsigned       _capacity    = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    template<typename T> PodArray<T>::PodArray(Allocator* allocator, unsigned initialCapacity)
    {
        _elements = Allocation::AllocateMemory<T>(initialCapacity);
        _capacity = initialCapacity;
    }

    template<typename T> PodArray<T>::~PodArray()
    {
        Allocation::FreeMemory(_elements);
    }

    template<typename T> template<typename T_INDEX> T& PodArray<T>::at(T_INDEX index)
    {
        assert((unsigned)index < _count);
        return _elements[index];
    }

    template<typename T> template<typename T_INDEX> const T& PodArray<T>::at(T_INDEX index) const
    {
        assert((unsigned)index < _count);
        return _elements[index];
    }

    template<typename T> template<typename T_INDEX> void PodArray<T>::remove(T_INDEX index)
    {
        assert((unsigned)index < _count);
        _count--;
        if (index != _count)
        {
            _elements[index] = _elements[_count];
        }
    }

    template<typename T> T& PodArray<T>::addLast()
    {
        reserve(_count+1, false);
        return _elements[_count++];
    }

    template<typename T> void PodArray<T>::setCount(unsigned count)
    {
        reserve(count, true);

        _count = count;
    }

    template<typename T> unsigned PodArray<T>::getCount()
    {
        return _count;
    }

    template<typename T> void PodArray<T>::reserve(unsigned capacity, bool exact)
    {
        if (capacity > _capacity)
        {
            if (!exact)
            {
                capacity = std::max(capacity, _capacity*2);
            }
            T* elements = Allocation::AllocateMemory<T>(capacity);
            memcpy(elements, _elements, _count);
            Allocation::FreeMemory(_elements);
            _elements = elements;
            _capacity = capacity;
        }
    }

}
