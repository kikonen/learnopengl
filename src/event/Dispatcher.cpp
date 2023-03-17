#include "Dispatcher.h"

#include "engine/UpdateContext.h"

namespace event {
    constexpr std::array<EventType, 2> EVENT_TYPES{
        EventType::node_add,
        EventType::node_added,
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
