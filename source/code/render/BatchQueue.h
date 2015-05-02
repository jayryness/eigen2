#pragma once
#include <cstdint>

#include "RenderBin.h"
#include "RenderPlan.h"

namespace eigen
{

    class Renderer;
    class RenderBatch;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // BatchQueue
    //
    // Accumulates all the batches (draw calls) to be issued via a RenderPlan.
    //
    // Use Renderer::openBatchQueue() to acquire one.
    //
    // TODO - Is it threadsafe, or one thread per batchQ and merge downsteam?
    //

    class BatchQueue
    {
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //

        void                commitBatch(RenderBatch* batch, const RenderBin* bin, float sortDepth);

        // TODO - plural RenderBin, defined by Effect (referenced by RenderBatch)
        // API then becomes:
        //void                commitBatch(RenderBatch* batch, const RenderBin::Set& binSelection, float sortDepth);

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
            BatchListEntry* head;
            int             count;
        };

        struct CachedSort
        {
            RenderBin::Set  binMask;
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

        static BatchQueue*    Create(Renderer* renderer, const RenderPlan* plan);

        SortCacheEntry&     findCachedDepthSort(const RenderBin::Set& bins) const; 
        SortCacheEntry&     findCachedPerfSort(const RenderBin::Set& bins) const; 

        BatchQueue*           _next               = nullptr;
        Renderer*           _renderer           = nullptr;
        Stage*              _stages             = nullptr;
        BatchList*          _batchLists         = nullptr;
        int8_t*             _buffer             = nullptr;
        int8_t*             _bufferEnd          = nullptr;
        SortCacheEntry*     _perfSortCache      = nullptr;
        SortCacheEntry*     _depthSortCache     = nullptr;
        RenderBin::Set      _binMask;
        unsigned            _stagesCount        = 0;
        unsigned            _sortCacheMask      = 0;
        unsigned            _binRangeStart      = 0;
        unsigned            _binRangeEnd        = 0;
    };

    inline BatchQueue::SortCacheEntry& BatchQueue::findCachedDepthSort(const RenderBin::Set& bins) const
    {
        unsigned hash = bins.hash();

        for (unsigned i = hash & _sortCacheMask; ; i = (i+1) & _sortCacheMask)
        {
            if (_depthSortCache[i].lookupKey == 0 || _depthSortCache[i].lookupKey == hash)
            {
                _depthSortCache[i].lookupKey = hash;
                return _depthSortCache[i];
            }
        }
    }

    inline BatchQueue::SortCacheEntry& BatchQueue::findCachedPerfSort(const RenderBin::Set& bins) const
    {
        unsigned hash = bins.hash();

        for (unsigned i = hash & _sortCacheMask; ; i = (i+1) & _sortCacheMask)
        {
            if (_perfSortCache[i].lookupKey == 0 || _perfSortCache[i].lookupKey == hash)
            {
                _perfSortCache[i].lookupKey = hash;
                return _depthSortCache[i];
            }
        }
    }

    inline bool BatchQueue::SortBatch::operator<(const SortBatch& other) const
    {
        return sortKey < other.sortKey;
    }

}
