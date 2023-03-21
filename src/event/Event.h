#pragma once

#include <array>
#include <variant>
#include <memory>

#include <glm/vec3.hpp>

#include "ki/uuid.h"


class UpdateContext;
class Node;
class NodeController;

namespace event {
    enum class Type {
        none = 0,
        node_add,
        node_added,
        node_change_parent,

        controller_add,

        animate_wait,
        animate_move,
        animate_rotate,
    };

    struct NodeAction {
        Node* target{ nullptr };
        uuids::uuid parentId;
    };

    struct ControlAction {
        int target{ 0 };
        NodeController* controller{ nullptr };
    };

    struct AnimateAction {
        int target{ 0 };

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
            ControlAction control;
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
