#pragma once

#include <memory>
#include <type_traits>

#include <glm/glm.hpp>

#include "ki/size.h"

namespace model
{
    class NodeType;
}

class NodeController;

enum class ControllerType : std::underlying_type_t<std::byte> {
    none,
    pawn,
    camera_zoom,
};

struct ControllerDefinition {
    ControllerType m_type{ ControllerType::none };

    int m_mode{ 0 };
    float m_speed{ 0.f };

    //ki::node_id m_targetId{ 0 };

    glm::vec3 m_direction{ 0.f, 0.f, 1.f };
    float m_distance{ 0.f };

    static std::unique_ptr<NodeController> createController(
        ControllerDefinition& definition);
};
