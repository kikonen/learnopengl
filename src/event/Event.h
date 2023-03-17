#pragma once

#include <array>
#include <variant>

#include "ki/uuid.h"


class UpdateContext;
class Node;

namespace event {
    enum class EventType {
        node_add,
        node_added,
        node_change_parent,
    };

    struct NodeEvent {
        Node* m_node{ nullptr };
        uuids::uuid m_parentId;
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
