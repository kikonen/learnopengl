#pragma once

#include <mutex>
#include <vector>

#include "eventpp/eventqueue.h"

#include "asset/Assets.h"

#include "Event.h"

class UpdateContexxt;

namespace event {
    class Queue final {
    public:
        Queue(const Assets& assets);

        void prepare();

        void dispatchEvents(const UpdateContext& ctx);

        void enqueu(const event::Event&& event) {
            m_queue.enqueue(event);
        }

        eventpp::EventQueue<
            EventType,
            void(const event::Event&),
            EventPolicies
        > m_queue;

    private:
        const Assets& m_assets;
        std::mutex m_lock;
    };
}
