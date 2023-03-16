#pragma once

#include <mutex>
#include <vector>

#include "asset/Assets.h"

#include "Event.h"

class UpdateContexxt;

namespace event {
    class Queue final {
    public:
        Queue(const Assets& assets);

        // USAGE:
        // event::NodeAdd evt{node};
        // m_eventQueue->addEvent(evt);
        //    OR
        // m_eventQueue->addEvent(event::NodeAdd {node});
        //
        void addEvent(const event::EventRef&& ref) {
            std::lock_guard<std::mutex> lock(m_lock);
            m_events.push_back(ref);
        }

        void dispatchEvents(const UpdateContext& ctx);

    private:
        const Assets& m_assets;
        std::mutex m_lock;

        std::vector<event::EventRef> m_events;
    };
}
