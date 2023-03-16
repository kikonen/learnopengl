#include "Queue.h"

#include "scene/UpdateContext.h"

namespace event {
    constexpr std::array<EventType, 2> EVENT_TYPES{
        EventType::node_add,
        EventType::node_added,
    };

    Queue::Queue(const Assets& assets)
        : m_assets(assets)
    {
    }

    void Queue::prepare()
    {
    }

    void Queue::dispatchEvents(const UpdateContext& ctx)
    {
        for (auto type : EVENT_TYPES) {
            m_queue.process();
        }
    }
}
