#include "memory.h"

namespace eigen
{

    inline BlockAllocator::Block* BlockAllocator::createBlock(Allocator* backing, unsigned capacity)
    {
        unsigned itemSize = (_allocationSize + 0xf) & ~0xf;
        itemSize += sizeof(Allocation);

        unsigned totalBytes = BlockAllocator::SizeOfBlock + itemSize*capacity;
        Block* block = (Block*)AllocateMemory<char>(backing, totalBytes);

        block->capacity = capacity;
        block->count    = 0;
        block->free     = (Allocation*)((char*)block + SizeOfBlock);
        block->next     = nullptr;

        Allocation* free = block->free;
        for (unsigned i = 0; i < capacity-1; i++)
        {
            free->_metadata = (char*)free + itemSize;
            (char*&)free += itemSize;
        }
        free->_metadata = nullptr;

        return block;
    }

    unsigned BlockAllocator::Block::sumCapacity() const
    {
        unsigned sum = capacity;
        for (Block* block = next; block; block = block->next)
        {
            sum += block->capacity;
        }
        return sum;
    }

    void BlockAllocator::addBlock()
    {
        int capacity = _head->sumCapacity();
        Allocator* allocator = _head->getAllocator();
        Block* block = createBlock(getBacking(), (capacity+3) & ~3);
        block->next = _head;
        _head = block;
    }

    void BlockAllocator::removeBlock(Block* block)
    {
        assert(block->count == 0);

        // walk the singly-linked list to find the preceeding block
        Block** prev;
        for (prev = &_head; *prev != block; prev = &(*prev)->next)
            ;

        *prev = block->next;

        FreeMemory(block);
    }

}
