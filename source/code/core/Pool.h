#pragma once

#include "Allocator.h"
#include <algorithm>

namespace eigen
{
    struct Block;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Pool
    //

    class Pool
    {
    public:
                        Pool();
                       ~Pool();

                        template<class T>
        void            initialize(Allocator* allocator, unsigned initialCapacity);

        int             getCount() const;
        int             getCapacity() const;

        void*           allocate();
        void            free(void* ptr);

                        template<class T>
        void            destroy(T* object);

        Allocator*      getAllocator() const;

    private:

        typedef void    (*BlockHandler)(Block**, Block*);

        void            addBlock();
        void            destroyBlock(Block* block);

        Block*         _block           = nullptr;
        BlockHandler   _blockHandler    = nullptr;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

    struct Block
    {
        struct ItemHead
        {
            union       {
            Block*      block;
            ItemHead*   next;
                        };
        };

        template<class T>
        struct Item   : public ItemHead
        {
            T           payload;
        };

        int             capacity;
        int             count;
        Block*          next;
        ItemHead*       free;

        Allocator*      getAllocator();
        int             sumCapacity() const;
        int             sumCount() const;
    };

    inline Allocator* Block::getAllocator()
    {
        return Allocation::From(this)->getAllocator();
    }

    inline int Block::sumCapacity() const
    {
        int sum = capacity;
        for (Block* block = next; block; block = block->next)
        {
            sum += block->capacity;
        }
        return sum;
    }

    inline int Block::sumCount() const
    {
        int sum = count;
        for (Block* block = next; block; block = block->next)
        {
            sum += block->count;
        }
        return sum;
    }

    template<class T> Block* CreateBlock(Allocator* allocator, int capacity) throw()
    {
        static_assert(sizeof(Block::ItemHead) == offsetof(Block::Item<T>, payload), "Bad struct alignment");

        Block*          block   = (Block*)Allocation::AllocateMemory<int8_t>(allocator, sizeof(Block) + sizeof(Block::Item<T>)*capacity);
        Block::Item<T>* free    = (Block::Item<T>*)(block + 1);
        block->capacity         = capacity;
        block->count            = 0;
        block->free             = free;
        block->next             = nullptr;

        for (int i = 0; i < capacity-1; i++)
        {
            free[i].next = free + i + 1;
        }
        free[capacity-1].next = nullptr;

        return block;
    }

    template<class T> void HandleBlock(Block** root, Block* block)
    {
        if (block)
        {
            // destroy block

            assert(block->count == 0);

            // walk the singly-linked list to find the preceeding block
            Block** prev;
            for (prev = root; *prev != block; prev = &(*prev)->next)
                ;

            *prev = block->next;

            Allocation::FreeMemory(block);
        }
        else
        {
            // add block

            int capacity = (*root)->sumCapacity();
            Allocator* allocator = (*root)->getAllocator();
            Block* block = CreateBlock<T>(allocator, capacity);
            block->next = *root;
            *root = block;
        }
    }

    inline Pool::Pool()
    {
    }

    inline Pool::~Pool()
    {
        while (_block)
        {
            _blockHandler(&_block, _block);     // destroy block
        }
    }

    template<class T> void Pool::initialize(Allocator* allocator, unsigned initialCapacity) throw()
    {
        assert(_block == nullptr);

        _block = CreateBlock<T>(allocator, initialCapacity);
        _blockHandler = HandleBlock<T>;
    }

    inline int Pool::getCount() const
    {
        return _block->sumCount();
    }

    inline int Pool::getCapacity() const
    {
        return _block->sumCapacity();
    }

    inline void* Pool::allocate() throw()
    {
        assert(_block != nullptr);              // must initialize the pool first
        Block::ItemHead* item = _block->free;
        _block->free = item->next;
        item->block = _block;

        if (++_block->count == _block->capacity)
        {
            assert(_block->free == nullptr);
            _blockHandler(&_block, nullptr);    // add block
        }

        return item + 1;
    }

    inline void Pool::free(void* ptr) throw()
    {
        Block::ItemHead *item = (Block::ItemHead*)ptr - 1;
        Block* block = item->block;
        item->next = block->free;
        block->free = item;

        // if the block is empty, and not the first block
        if (--block->count == 0 && block != _block)
        {
            _blockHandler(&_block, block);      // destroy it
        }
    }

    template<class T> void Pool::destroy(T* object) throw()
    {
        if (object != nullptr)
        {
            object->~T();
            free(object);
        }
    }

    inline Allocator* Pool::getAllocator() const
    {
        return _block->getAllocator();
    }
}
