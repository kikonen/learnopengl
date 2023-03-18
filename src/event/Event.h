#pragma once

#include <array>
#include <variant>

#include "ki/uuid.h"


class UpdateContext;
class Node;

namespace event {
    enum class EventType {
        none = 0,
        node_add,
        node_added,
        node_change_parent,
        animate_move,
        animate_rotate,
    };

    struct NodeEvent {
        Node* m_node{ nullptr };
        uuids::uuid m_parentId;
    };

    struct AnimateEvent {
        int id;
    };

    struct Event {
        EventType m_type;
        union {
            NodeEvent nodeEvent;
            AnimateEvent animateEvent;
        } ref;
    };

    struct EventPolicies
    {
        static EventType getEvent(const Event& e) {
            return e.m_type;
        }
    };
}
