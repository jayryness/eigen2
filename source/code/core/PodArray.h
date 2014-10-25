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
        T&              at(T_INDEX index);

                        template<typename T_INDEX>
        const T&        at(T_INDEX index) const;

        T&              append();

        void            setCount(unsigned count);
        unsigned        getCount();

        void            reserve(unsigned capacity);

    private:

        T*             _elements    = nullptr;
        unsigned       _count       = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    template<typename T> PodArray<T>::PodArray(Allocator* allocator, unsigned initialCapacity)
    {
        _elements = Allocation::AllocateMemory<T>(initialCapacity);
    }

    template<typename T> PodArray<T>::~PodArray()
    {
        Allocation::FreeMemory(_elements);
    }

    template<typename T> template<typename T_INDEX> T& PodArray<T>::at(T_INDEX index)
    {
        assert(index < _count);
        return _elements[index];
    }

    template<typename T> template<typename T_INDEX> const T& PodArray<T>::at(T_INDEX index) const
    {
        assert(index < _count);
        return _elements[index];
    }

    template<typename T> T& PodArray<T>::append()
    {
        reserve(_count+1);
        return _elements[_count++];
    }

    template<typename T> void PodArray<T>::setCount(unsigned count)
    {
        reserve(count);

        _count = count;
    }

    template<typename T> unsigned PodArray<T>::getCount()
    {
        return _count;
    }

    template<typename T> void PodArray<T>::reserve(unsigned capacity)
    {
        Allocation* allocation = Allocation::From(_elements);
        unsigned curCapacity = allocation->getArrayLength();

        if (capacity > curCapacity)
        {
            T* elements = Allocation::AllocateMemory<T>(capacity);
            memmove(elements, _elements, _count);
            allocation->destroy();
            _elements = elements;
        }
    }

}
