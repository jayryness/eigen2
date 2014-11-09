#pragma once
#include <cstdint>

#include "RenderPort.h"
#include "RenderPlan.h"

namespace eigen
{

    class Renderer;
    struct Batch;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Worklist
    //
    // Accumulates all the batches (draw calls) to be issued via a RenderPlan.
    //
    // Use Renderer::openWorklist() to acquire one.
    //
    // TODO - Is it threadsafe, or one thread per worklist and merge downsteam?
    //

    class Worklist
    {
    public:

        void                commitBatch(Batch* batch, const RenderPort* port, float sortDepth);
        void                finish();

    protected:

        struct Item
        {
            Item*           next;
            float           sortDepth;
            unsigned        performanceSortKey;
            //Batch         batch;    // shallow copy, variable # of paramblocks follow in memory
        };

        struct SortBatch
        {
            uint64_t        sortKey;
            Batch*          batch;

            bool            operator<(const SortBatch& other) const;
        };

        struct Slot
        {
            Item*           head;   // could make this atomic
            int             count;  // and this
            //SortBatch*      performanceSorted;
        };

        struct CachedSort
        {
            RenderPort::Set ports;
            SortBatch*      batches;
            unsigned        count;
        };

        struct SortCacheEntry
        {
            unsigned        hash;
            CachedSort*     cached;
        };

        enum
        {
                            ChunkSize = 16*1024,
        };
                            friend class Renderer;
                            friend class WorkCoordinator;

        static Worklist*    Create(Renderer* renderer, const RenderPlan* plan);

        SortCacheEntry&     findCachedSort(unsigned hash) const; 

        Worklist*           _next           = nullptr;
        Renderer*           _renderer       = nullptr;
        Stage*              _stages         = nullptr;
        Slot*               _slots          = nullptr;
        int8_t*             _buffer         = nullptr;
        int8_t*             _bufferEnd      = nullptr;
        SortCacheEntry*     _sortCache      = nullptr;
        RenderPort::Set     _ports;
        unsigned            _stagesCount    = 0;
        unsigned            _sortCacheMask  = 0;
        unsigned            _portRangeStart = 0;
        unsigned            _portRangeEnd   = 0;
    };

    inline Worklist::SortCacheEntry& Worklist::findCachedSort(unsigned hash) const
    {
        for (unsigned i = hash & _sortCacheMask; ; i = (i+1) & _sortCacheMask)
        {
            if (_sortCache[i].hash == 0 || _sortCache[i].hash == hash)
            {
                _sortCache[i].hash = hash;
                return _sortCache[i];
            }
        }
    }

    inline bool Worklist::SortBatch::operator<(const SortBatch& other) const
    {
        return sortKey < other.sortKey;
    }

}
