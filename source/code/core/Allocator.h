#pragma once

#include <new>
#include <atomic>
#include <cassert>

namespace eigen
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocator
    //

    class Allocator
    {
    public:

        virtual void*       allocate(unsigned bytes) = 0;
        virtual void        free(void* ptr) = 0;
    };

    template<class T> T*    AllocateMemory(Allocator* allocator, unsigned arrayLength);
    template<class T> T*    AllocateMemory(Allocator* allocator, unsigned arrayLength, void* metadata);
    void                    FreeMemory(void*);
    template<class T> void  Delete(T* obj);

    typedef void            (*DeleteFunc)(void* obj);

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Allocation
    //

    struct Allocation
    {
        static Allocation*  From(void*);

        void                destroy();
        void*               getMemory() const;

        void*              _metadata;
        Allocator*         _allocator;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // BlockAllocator
    //

    class BlockAllocator  : public Allocator
    {
    public:
                           ~BlockAllocator();

        void                initialize(Allocator* backing, unsigned allocationSize, unsigned initialReservations);

        void*               allocate(unsigned ignored)  final;
        void                free(void* ptr)             final;

        Allocator*          getBacking() const;

    private:

        struct Block
        {
            Block*          next;
            Allocation*     free;
            unsigned        capacity;
            unsigned        count;

            Allocator*      getAllocator();
            unsigned        sumCapacity() const;
        };
        enum {              SizeOfBlock = (sizeof(Block)+0xf) & ~0xf };

        Block*              createBlock(Allocator* backing, unsigned capacity);
        void                addBlock();
        void                removeBlock(Block*);

        Block*             _head            = 0;
        unsigned           _allocationSize  = 0;

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

        std::atomic<unsigned> _count = 0;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////

    inline Allocation* Allocation::From(void* memory)
    {
        Allocation* allocation = (Allocation*)memory;
        allocation--;
        return allocation;
    }

    inline void Allocation::destroy()
    {
        _allocator->free(this);
    }

    inline void* Allocation::getMemory() const
    {
        return (void*)(this+1);
    }

    template<class T> T* AllocateMemory(Allocator* allocator, unsigned arrayLength, void* metadata)
    {
        Allocation* allocation = (Allocation*)allocator->allocate(sizeof(Allocation) + sizeof(T)*arrayLength);

        allocation->_metadata  = metadata;
        allocation->_allocator = allocator;

        return (T*)allocation->getMemory();
    }

    template<class T> T* AllocateMemory(Allocator* allocator, unsigned arrayLength)
    {
        Allocation* allocation = (Allocation*)allocator->allocate(sizeof(Allocation) + sizeof(T)*arrayLength);

        allocation->_allocator = allocator;

        return (T*)allocation->getMemory();
    }

    inline void FreeMemory(void* memory)
    {
        if (memory)
        {
            Allocation* allocation = Allocation::From(memory);
            allocation->destroy();
        }
    }

    template<class T> void Delete(T* object)
    {
        if (object)
        {
            object->~T();
            Allocation* allocation = Allocation::From(object);
            allocation->destroy();
        }
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

    inline Allocator* BlockAllocator::Block::getAllocator()
    {
        assert(this);
        return Allocation::From(this)->_allocator;
    }

    inline BlockAllocator::~BlockAllocator()
    {
        while (_head)
        {
            removeBlock(_head);
        }
    }

    inline void BlockAllocator::initialize(Allocator* backing, unsigned allocationSize, unsigned initialReservations)
    {
        assert(_head == nullptr);   // already initialized
        _allocationSize = allocationSize;
        _head = createBlock(backing, initialReservations);
    }

    inline Allocator* BlockAllocator::getBacking() const
    {
        assert(_head);  // must initialize() first
        return _head->getAllocator();
    }

    inline void* BlockAllocator::allocate(unsigned bytes)
    {
        assert(_head);  // must initialize() first

        Allocation* allocation = _head->free;

        _head->count++;
        _head->free = (Allocation*)allocation->_metadata;   // next free
        allocation->_metadata = _head;                      // now metadata points back to block instead of next

        if (_head->count == _head->capacity)
        {
            assert(_head->free->_metadata == nullptr);
            addBlock();
        }

        return allocation;
    }

    inline void BlockAllocator::free(void* ptr)
    {
        Allocation* allocation = (Allocation*)ptr;
        Block* block = (Block*)allocation->_metadata;

        allocation->_metadata = block->free;
        block->free = allocation;
        block->count--;

        // if the block is empty, and not the first block
        if (block->count == 0 && block != _head)
        {
            removeBlock(block);
        }
    }

}
