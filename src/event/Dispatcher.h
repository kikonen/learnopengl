#pragma once

#include <atomic>
#include <vector>

#include "eventpp/eventqueue.h"

#include "asset/Assets.h"

#include "Event.h"


namespace event {
    class Dispatcher final {
    public:
        Dispatcher(const Assets& assets);

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

    private:
        const Assets& m_assets;

        eventpp::EventQueue<
            Type,
            void(const event::Event&),
            EventPolicies
        > m_queue;
    };
}
