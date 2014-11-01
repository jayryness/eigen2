#pragma once

namespace eigen
{
    class Worklist;

    struct WorkCoordinator
    {
                            WorkCoordinator(Worklist* head);

        void                operator()();

        Worklist*           _head;

    };

    inline WorkCoordinator::WorkCoordinator(Worklist* head)
        : _head(head)
    {
    }

    inline void WorkCoordinator::operator()()
    {
    }
}
