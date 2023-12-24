#include "Dispatcher.h"

namespace event {
    //constexpr std::array<Type, 8> EVENT_TYPES{
    //    Type::none,

    //    Type::node_add,
    //    Type::node_added,
    //    Type::node_change_parent,

    //    Type::controller_add,

    //    Type::animate_wait,
    //    Type::animate_move,
    //    Type::animate_rotate,
    //};

    Dispatcher::Dispatcher(const Assets& assets)
        : m_assets(assets)
    {
    }

    void Dispatcher::prepare()
    {
    }

    void Dispatcher::dispatchEvents()
    {
        //for (auto type : EVENT_TYPES) {
        //    m_queue.process();
        //}
        m_queue.process();
    }
}
