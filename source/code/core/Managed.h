#pragma once

#include "RefCounted.h"
#include "Allocator.h"
#include <atomic>

namespace eigen
{

    template<class T> class Managed;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Manager
    //

    template<class T> class Manager
    {
    public:
                                    Manager() {}
                                   ~Manager();

                                    template<class T_CONCRETE>
        void                        initialize(Allocator* allocator, unsigned initialCapacity);
        void                        cleanup();

                                    template<class T_CONCRETE>
        T_CONCRETE*                 create();
        //void                        discard(Managed<T>* obj);   // destruction deferred until next destroyGarbage()

        unsigned                    getCount() const;
        Allocator*                  getAllocator() const;

        //Managed<T>*               getFirst();
        //Managed<T>*               getNext(Managed<T>* prev);

        //void                        destroyGarbage();

    protected:

        typedef void                (*DeleteFunc)(void*);

        BlockAllocator              _itemAllocator;
        //DeleteFunc                  _deleteFunc     = nullptr;
        //std::atomic<Managed<T>*>    _garbage        = nullptr;
        unsigned                    _count          = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Managed
    //

    template<class T> class Managed : public RefCounted<T>
    {
    public:

        class Manager<T>*   getManager() const;

    protected:

        friend class        Manager<T>;

        union               {
        Manager<T>*        _manager = nullptr;
        Managed*           _next;
                            };
    };

    ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////

    template<class T> Manager<T>::~Manager()
    {
        cleanup();
    }

    template<class T> void Manager<T>::cleanup()
    {
        //assert(_count == 0);
        //destroyGarbage();
    }

    template<class T> template<class T_CONCRETE> void Manager<T>::initialize(Allocator* allocator, unsigned initialCapacity)
    {
        //_deleteFunc = (DeleteFunc)Delete<T_CONCRETE>;
        _itemAllocator.initialize(allocator, sizeof(T_CONCRETE), initialCapacity);
    }

    template<class T> template<class T_CONCRETE> T_CONCRETE* Manager<T>::create() throw()
    {
        T_CONCRETE* obj = new(AllocateMemory<T_CONCRETE>(&_itemAllocator, 1)) T_CONCRETE();
        obj->_manager = this;
        _count++;
        return obj;
    }

    //template<class T> void Manager<T>::discard(Managed<T>* obj) throw()
    //{
    //    obj->_next = _garbage.exchange(obj, std::memory_order_relaxed);
    //    _count--;
    //}

    template<class T> unsigned Manager<T>::getCount() const
    {
        return _count;
    }

    template<class T> Allocator* Manager<T>::getAllocator() const
    {
        return _itemAllocator.getBacking();
    }

    //template<class T> void Manager<T>::destroyGarbage()
    //{
    //    Managed<T>* garbage = _garbage.exchange(nullptr, std::memory_order_relaxed);

    //    while (garbage != nullptr)
    //    {
    //        Managed<T>* next = garbage->_next;
    //        garbage->_manager = this;   // in case of getManager() call in dtor (remember _next and _manager are union)
    //        _deleteFunc(garbage);
    //        garbage = next;
    //    }
    //}

    template<class T> Manager<T>* Managed<T>::getManager() const
    {
        return _manager;
    }

}
