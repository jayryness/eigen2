#pragma once

namespace eigen
{
    class Worklist;
    class Renderer;

    class WorkCoordinator
    {
    public:
                            WorkCoordinator() {}
                            WorkCoordinator(Renderer& renderer, Worklist* head);

        void                operator()();

        Worklist*           _head = nullptr;

    };

    inline void WorkCoordinator::operator()()
    {
    }
}
