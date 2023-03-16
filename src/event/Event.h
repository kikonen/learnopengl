#pragma once

#include <array>
#include <variant>

class UpdateContext;
class Node;

namespace event {
    enum class EventType {
        node_add,
        node_added,
    };

    struct NodeEvent {
        Node* m_node{ nullptr };
    };

    struct Event {
        EventType m_type;
        union {
            NodeEvent nodeEvent;
        } ref;
    };

    struct EventPolicies
    {
        static EventType getEvent(const Event& e) {
            return e.m_type;
        }
    };
}
