#pragma once
#include <cstdint>

#include "RenderPort.h"
#include "RenderPlan.h"

namespace eigen
{

    class Renderer;
    class RenderBatch;

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

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //

        void                commitBatch(RenderBatch* batch, const RenderPort* port, float sortDepth);
        void                finish();

        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    protected:

        struct BatchListEntry
        {
            BatchListEntry* next;
            float           sortDepth;
            unsigned        performanceSortKey;
            //Batch         batch;    // shallow copy, variable # of paramblocks follow in memory
        };

        struct SortBatch
        {
            uint64_t        sortKey;
            RenderBatch*    batch;

            bool            operator<(const SortBatch& other) const;
        };

        struct BatchList
        {
            BatchListEntry* head;   // could make this atomic
            int             count;  // and this
        };

        struct CachedSort
        {
            RenderPort::Set ports;
            SortBatch*      batches;
            unsigned        count;
        };

        struct SortCacheEntry
        {
            unsigned        lookupKey;
            CachedSort*     cached;
        };

        enum
        {
                            ChunkSize = 16*1024,
        };
                            friend class Renderer;
                            friend class RenderDispatch;

        static Worklist*    Create(Renderer* renderer, const RenderPlan* plan);

        SortCacheEntry&     findCachedDepthSort(const RenderPort::Set& ports) const; 
        SortCacheEntry&     findCachedPerfSort(const RenderPort::Set& ports) const; 

        Worklist*           _next               = nullptr;
        Renderer*           _renderer           = nullptr;
        Stage*              _stages             = nullptr;
        BatchList*          _batchLists         = nullptr;
        int8_t*             _buffer             = nullptr;
        int8_t*             _bufferEnd          = nullptr;
        SortCacheEntry*     _perfSortCache      = nullptr;
        SortCacheEntry*     _depthSortCache     = nullptr;
        RenderPort::Set     _ports;
        unsigned            _stagesCount        = 0;
        unsigned            _sortCacheMask      = 0;
        unsigned            _portRangeStart     = 0;
        unsigned            _portRangeEnd       = 0;
    };

    inline Worklist::SortCacheEntry& Worklist::findCachedDepthSort(const RenderPort::Set& ports) const
    {
        unsigned hash = ports.hash();

        for (unsigned i = hash & _sortCacheMask; ; i = (i+1) & _sortCacheMask)
        {
            if (_depthSortCache[i].lookupKey == 0 || _depthSortCache[i].lookupKey == hash)
            {
                _depthSortCache[i].lookupKey = hash;
                return _depthSortCache[i];
            }
        }
    }

    inline Worklist::SortCacheEntry& Worklist::findCachedPerfSort(const RenderPort::Set& ports) const
    {
        unsigned hash = ports.hash();

        for (unsigned i = hash & _sortCacheMask; ; i = (i+1) & _sortCacheMask)
        {
            if (_perfSortCache[i].lookupKey == 0 || _perfSortCache[i].lookupKey == hash)
            {
                _perfSortCache[i].lookupKey = hash;
                return _depthSortCache[i];
            }
        }
    }

    inline bool Worklist::SortBatch::operator<(const SortBatch& other) const
    {
        return sortKey < other.sortKey;
    }

}
