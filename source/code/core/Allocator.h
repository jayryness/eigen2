#pragma once

#include <new>
#include <atomic>

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocator
    //

    class Allocator
    {
    protected:
        virtual void*       allocate(unsigned bytes) = 0;
        virtual void        free(void* ptr) = 0;

                            friend class Allocation;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocation
    //

    class Allocation
    {
    public:
                            template<class T>
        static T*           AllocateMemory(Allocator* allocator, int arrayLength=1);
        static void         FreeMemory(void*);
                            template<class T>
        static Allocation*  Create(Allocator* allocator, int arrayLength=1);
        static Allocation*  From(void*);

        void                destroy();

        int                 getArrayLength() const;
        Allocator*          getAllocator() const;
        void*               getMemory() const;

    private:

        int32_t             _arrayLength;
        Allocator*          _allocator;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Mallocator
    //

    class Mallocator      : public Allocator
    {
    public:
        static Mallocator*  Get();

                           ~Mallocator();

        unsigned            getCount() const;

    protected:
        void*               allocate(unsigned bytes) override;
        void                free(void* ptr) override;

        std::atomic<unsigned>  _count = 0;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    inline Allocation* Allocation::From(void* memory)
    {
        Allocation* allocation = (Allocation*)memory;
        allocation--;
        return allocation;
    }

    template<class T> Allocation* Allocation::Create(Allocator* allocator, int arrayLength)
    {
        Allocation* allocation = (Allocation*)allocator->allocate(sizeof(Allocation) + sizeof(T)*arrayLength);

        allocation->_arrayLength = arrayLength;
        allocation->_allocator   = allocator;
        return allocation;
    }

    inline void Allocation::destroy()
    {
        _allocator->free(this);
    }

    inline int Allocation::getArrayLength() const
    {
        return _arrayLength;
    }

    inline Allocator* Allocation::getAllocator() const
    {
        return _allocator;
    }

    inline void* Allocation::getMemory() const
    {
        return (void*)(this+1);
    }

    inline void Allocation::FreeMemory(void* memory)
    {
        if (memory)
        {
            Allocation* allocation = From(memory);
            allocation->destroy();
        }
    }

    template<class T> inline T* Allocation::AllocateMemory(Allocator* allocator, int arrayLength)
    {
        Allocation* allocation = Allocation::Create<T>(allocator, arrayLength);
        return (T*)allocation->getMemory();
    }

    inline Mallocator* Mallocator::Get() throw()
    {
        static Mallocator s_mallocator;
        return &s_mallocator;
    }

    inline Mallocator::~Mallocator()
    {
        assert(getCount() == 0);
    }

    inline unsigned Mallocator::getCount() const
    {
        return _count.load(std::memory_order_relaxed);
    }

    inline void* Mallocator::allocate(unsigned bytes)
    {
        _count.fetch_add(+1, std::memory_order_relaxed);
        return ::malloc(bytes);
    }

    inline void Mallocator::free(void* ptr)
    {
        _count.fetch_add(-1, std::memory_order_relaxed);
        ::free(ptr);
    }

}
