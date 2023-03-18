#pragma once

#include <array>
#include <variant>

#include <glm/vec3.hpp>

#include "ki/uuid.h"


class UpdateContext;
class Node;

namespace event {
    enum class Type {
        none = 0,
        node_add,
        node_added,
        node_change_parent,

        animate_wait,
        animate_move,
        animate_rotate,
    };

    struct NodeAction {
        Node* target{ nullptr };
        uuids::uuid parentId;
    };

    struct AnimateAction {
        int target;

        int after{ 0 };
        float duration{ 0 };
        bool relative{ true };
        glm::vec3 data{ 0.f };
    };

    struct Event {
        Type type;
        int id;

        union {
            NodeAction node;
            AnimateAction animate;
        } body;
    };

    struct EventPolicies
    {
        static Type getEvent(const Event& e) {
            return e.type;
        }
    };
}
