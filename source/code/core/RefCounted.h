#pragma once

#include <atomic>

namespace eigen
{

    template<class T> struct RefPtr
    {
        RefPtr() : ptr(0) {}
        RefPtr(T* ptr_) : ptr(ptr_)
        {
            if (ptr)
            {
                ChangeRef((RefCounted<T>*)ptr, +1);
            }
        }

        RefPtr(const RefPtr& other) : RefPtr(other.ptr)
        {
        }

        ~RefPtr()
        {
            if (ptr)
            {
                ChangeRef((RefCounted<T>*)ptr, -1);
            }
        }

        void operator=(RefPtr other)
        {
            swap(other);
        }

        operator T*()
        {
            return ptr;
        }

        template<class T2> void swap(RefPtr<T2>& other)
        {
            T* temp = ptr;
            ptr = static_cast<T*>(other.ptr);
            other.ptr = static_cast<T2*>(temp);
        }

        T* ptr;
    };

    template<class T> class RefCounted
    {
    public:

        friend void AssignRef(T*& dest, T*const src)
        {
            (RefPtr<T>&)dest = src;
        }

        friend void AddRef(T* ptr)
        {
            if (ptr)
            {
                ChangeRef(ptr, +1);
            }
        }

        friend void ReleaseRef(T* ptr)
        {
            if (ptr)
            {
                ChangeRef(ptr, -1);
            }
        }

        bool isLastRef() const
        {
            return ptr->_refCount.load(std::memory_order_relaxed) == 1;
        }

    protected:

        RefCounted()
        {
            _refCount.store(0, std::memory_order_relaxed);
        }

    private:

        friend void ChangeRef(RefCounted* ptr, int delta)
        {
            if (ptr->_refCount.fetch_add(delta, std::memory_order_relaxed) <= -delta)
            {
                DestroyRefCounted((T*) ptr);
            }
        }

        friend void DestroyRefCounted(T*);
        std::atomic_int _refCount;
    };

}
