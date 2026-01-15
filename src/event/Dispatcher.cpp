#include "Dispatcher.h"

namespace event {
    Dispatcher::Dispatcher()
    {
    }

    void Dispatcher::dispatchEvents()
    {
        // Apply pending listener changes first (lock-free)
        applyPendingChanges();

        // Dispatch events without holding any lock
        event::Event event;
        while (m_queue.try_dequeue(event)) {
            for (auto& entry : m_handlers[event.type]) {
                entry.second(event);
            }
        }
    }

    void Dispatcher::applyPendingChanges()
    {
        // Process pending adds
        PendingAdd add;
        while (m_pendingAdds.try_dequeue(add)) {
            m_handlers[add.type].push_back({ add.handle, std::move(add.handler) });
        }

        // Process pending removes
        PendingRemove rem;
        while (m_pendingRemoves.try_dequeue(rem)) {
            auto& handlers = m_handlers[rem.type];
            const auto it = std::remove_if(
                handlers.begin(),
                handlers.end(),
                [&rem](const Entry& entry) {
                    return entry.first == rem.handle;
                });
            handlers.erase(it, handlers.end());
        }
    }
}
