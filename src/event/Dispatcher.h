#pragma once

#include <atomic>
#include <vector>

#include "eventpp/eventqueue.h"
#include "eventpp/eventdispatcher.h"

#include "Event.h"


namespace event {
    using Queue = eventpp::EventQueue<
        Type,
        void(const event::Event&),
        EventPolicies
    >;
    using Handle = Queue::Handle;

    class Dispatcher final {
    public:
        Dispatcher();

        void prepare();

        void dispatchEvents();

        inline void send(Event& evt)
        {
            m_queue.enqueue(evt);
        }

        template <typename Callback>
        Handle addListener(event::Type type, Callback&& callback)
        {
            return m_queue.appendListener(type, std::forward<Callback>(callback));
        }

        void removeListener(event::Type type, Handle handle)
        {
            m_queue.removeListener(type, handle);
        }

    private:
        Queue m_queue;
    };
}
