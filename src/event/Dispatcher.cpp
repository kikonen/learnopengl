#include "Dispatcher.h"

#include "engine/UpdateContext.h"

namespace event {
    constexpr std::array<Type, 7> EVENT_TYPES{
        Type::none,
        Type::node_add,
        Type::node_added,
        Type::node_change_parent,

        Type::animate_wait,
        Type::animate_move,
        Type::animate_rotate,
    };

    Dispatcher::Dispatcher(const Assets& assets)
        : m_assets(assets)
    {
    }

    void Dispatcher::prepare()
    {
    }

    void Dispatcher::dispatchEvents(const UpdateContext& ctx)
    {
        for (auto type : EVENT_TYPES) {
            m_queue.process();
        }
    }
}
