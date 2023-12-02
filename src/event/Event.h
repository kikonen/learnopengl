#pragma once

#include <array>
#include <variant>
#include <memory>

#include <glm/vec3.hpp>

#include "ki/size.h"
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
        node_select,
        node_activate,

        // NOTE KI allow camera to vary independent of active node
        camera_activate,

        controller_add,

        animate_wait,
        animate_move,
        animate_rotate,

        scene_loaded,
    };

    struct NodeAction {
        Node* target{ nullptr };
        uuids::uuid parentId;
    };

    struct ControlAction {
		ki::object_id target{ 0 };
        NodeController* controller{ nullptr };
    };

    struct AnimateAction {
		ki::object_id target{ 0 };

        int after{ 0 };
        float duration{ 0 };
        bool relative{ true };
        glm::vec3 data{ 0.f };
    };

    struct Event {
        Type type;
        ki::event_id id;

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
