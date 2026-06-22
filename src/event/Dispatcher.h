#pragma once

#include <atomic>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <moodycamel/concurrentqueue.h>

#include "util/Ref.h"

#include "Event.h"

namespace event {
    using Handler = std::function<void(const Event&)>;
    using Handle = uint32_t;
    using Entry = std::pair<Handle, Handler>;

    class Dispatcher final : public util::RefCounted<> {
    public:
        Dispatcher();
        ~Dispatcher();

        void dispatchEvents();

        inline void send(Event& evt)
        {
            m_queue.enqueue(std::move(evt));
        }

        inline void send(Event&& evt)
        {
            m_queue.enqueue(std::forward<Event>(evt));
        }

        // Defer arbitrary work to this dispatcher's drain point (= its thread).
        // The task rides the SAME queue as events, so its order relative to
        // events is the queue's per-producer FIFO, not a cross-queue race.
        inline void invokeLater(event::Task task)
        {
            Event evt{ Type::invoke };
            evt.attach()->task = std::move(task);
            m_queue.enqueue(std::move(evt));
        }

        Handle addListener(event::Type type, Handler handler)
        {
            auto handle = m_handleIndex.fetch_add(1, std::memory_order_relaxed);
            m_pendingAdds.enqueue({ type, handle, std::move(handler) });
            return handle;
        }

        void removeListener(event::Type type, Handle handle)
        {
            m_pendingRemoves.enqueue({ type, handle });
        }

    private:
        void applyPendingChanges();

    private:
        struct PendingAdd {
            event::Type type;
            Handle handle;
            Handler handler;
        };

        struct PendingRemove {
            event::Type type;
            Handle handle;
        };

        std::atomic<uint32_t> m_handleIndex{ 1 };
        std::unordered_map<event::Type, std::vector<Entry>> m_handlers;

        moodycamel::ConcurrentQueue<Event> m_queue;
        moodycamel::ConcurrentQueue<PendingAdd> m_pendingAdds;
        moodycamel::ConcurrentQueue<PendingRemove> m_pendingRemoves;
    };
}
