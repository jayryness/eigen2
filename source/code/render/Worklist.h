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

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // User API
        //

        void                commitBatch(Batch* batch, const RenderPort* port, float sortDepth);
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
            Batch*          batch;

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
                            friend class WorkCoordinator;

        static Worklist*    Create(Renderer* renderer, const RenderPlan* plan);

        SortCacheEntry&     findCachedDepthSort(const RenderPort::Set& ports) const; 
        SortCacheEntry&     findCachedPerfSort(unsigned portIndex) const; 

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
        unsigned            _depthSortCacheMask = 0;
        unsigned            _portRangeStart     = 0;
        unsigned            _portRangeEnd       = 0;
    };

    inline Worklist::SortCacheEntry& Worklist::findCachedDepthSort(const RenderPort::Set& ports) const
    {
        unsigned hash = ports.hash();

        for (unsigned i = hash & _depthSortCacheMask; ; i = (i+1) & _depthSortCacheMask)
        {
            if (_depthSortCache[i].lookupKey == 0 || _depthSortCache[i].lookupKey == hash)
            {
                _depthSortCache[i].lookupKey = hash;
                return _depthSortCache[i];
            }
        }
    }

    inline Worklist::SortCacheEntry& Worklist::findCachedPerfSort(unsigned portIndex) const
    {
        assert(_portRangeStart <= portIndex && portIndex < _portRangeEnd);
        assert(_perfSortCache[portIndex].lookupKey == 0 || _perfSortCache[portIndex].lookupKey == portIndex);
        _perfSortCache[portIndex].lookupKey = portIndex;
        return _perfSortCache[portIndex];
    }

    inline bool Worklist::SortBatch::operator<(const SortBatch& other) const
    {
        return sortKey < other.sortKey;
    }

}
