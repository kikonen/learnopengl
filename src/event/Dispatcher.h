#pragma once

#include <atomic>
#include <vector>

#include "eventpp/eventqueue.h"

#include "asset/Assets.h"

#include "Event.h"

class UpdateContexxt;

namespace event {
    class Dispatcher final {
    public:
        Dispatcher(const Assets& assets);

        void prepare();

        void dispatchEvents(const UpdateContext& ctx);

        inline int send(Event& evt)
        {
            evt.id = nextID();
            m_queue.enqueue(evt);
            return evt.id;
        }

        template <typename ...Params>
        void addListener(Params&&... params)
        {
            m_queue.appendListener(std::forward<Params>(params)...);
        }

    private:
        inline int nextID() {
            return m_baseId++;
        }

    private:
        const Assets& m_assets;

        std::atomic<int> m_baseId{ 1 };

        eventpp::EventQueue<
            Type,
            void(const event::Event&),
            EventPolicies
        > m_queue;
    };
}
