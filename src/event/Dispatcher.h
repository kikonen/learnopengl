#pragma once

#include <atomic>
#include <vector>

#include "eventpp/eventqueue.h"

#include "Event.h"


namespace event {
    class Dispatcher final {
    public:
        Dispatcher();

        void prepare();

        void dispatchEvents();

        inline void send(Event& evt)
        {
            m_queue.enqueue(evt);
        }

        template <typename ...Params>
        void addListener(Params&&... params)
        {
            m_queue.appendListener(std::forward<Params>(params)...);
        }

    private:
        eventpp::EventQueue<
            Type,
            void(const event::Event&),
            EventPolicies
        > m_queue;
    };
}
