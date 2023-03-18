#include "Dispatcher.h"

#include "engine/UpdateContext.h"

namespace event {
    constexpr std::array<EventType, 7> EVENT_TYPES{
        EventType::none,
        EventType::node_add,
        EventType::node_added,
        EventType::node_change_parent,

        EventType::animate_wait,
        EventType::animate_move,
        EventType::animate_rotate,
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
