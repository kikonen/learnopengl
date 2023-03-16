#include "Queue.h"

#include "scene/UpdateContext.h"

namespace event {
    Queue::Queue(const Assets& assets)
        : m_assets(assets)
    {
    }

    struct DispatchVisitor {
        const UpdateContext* ctx{ nullptr };

        void operator()(NodeAdd& event) {
            event.dispatch(*ctx);
        }
    };

    void Queue::dispatchEvents(const UpdateContext& ctx)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        if (m_events.empty()) return;

        static event::DispatchVisitor visitor{};
        visitor.ctx = &ctx;

        for (auto& ref : m_events) {
            std::visit(visitor, ref);
        }
        m_events.clear();
    }
}
