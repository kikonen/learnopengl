#pragma once

#include <array>
#include <variant>

#include <glm/vec3.hpp>

#include "ki/uuid.h"


class UpdateContext;
class Node;

namespace event {
    enum class EventType {
        none = 0,
        node_add,
        node_added,
        node_change_parent,

        animate_wait,
        animate_move,
        animate_rotate,
    };

    struct NodeEvent {
        Node* target{ nullptr };
        uuids::uuid parentId;
    };

    struct AnimateEvent {
        int target;

        int after{ 0 };
        float duration{ 0 };
        bool relative{ true };
        glm::vec3 data{ 0.f };
    };

    struct Event {
        EventType type;
        int id;

        union {
            NodeEvent node;
            AnimateEvent animate;
        } body;
    };

    struct EventPolicies
    {
        static EventType getEvent(const Event& e) {
            return e.type;
        }
    };
}
