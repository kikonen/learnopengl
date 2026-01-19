#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Axis.h"

#include "physics/Category.h"
#include "physics/size.h"

// @see physics/Body.h
struct BodyDefinition {
    // NOTE KI default {0} means "inherit from shape" - handled in PhysicsLoader
    glm::vec3 m_size{ 0.f };

    util::Axis m_baseAxis{ util::Axis::y };
    util::Front m_baseFront{ util::Front::z };
    glm::vec3 m_baseAdjust{ 0.f };

    glm::vec3 m_linearVelocity{ 0.f };
    glm::vec3 m_angularVelocity{ 0.f };

    glm::vec3 m_axis{ 0.f, 1.f, 0.f };

    float m_maxAngulerVelocity{ 100.f };
    float m_density{ 1.f };

    physics::BodyType m_type{ physics::BodyType::none };

    bool m_forceAxis : 1 { false };
    bool m_kinematic : 1 { false };
};

// @see physics/Shape.h
struct ShapeDefinition {
    bool m_enabled{ false };

    glm::vec3 m_size{ 0.5f };

    util::Axis m_baseAxis{ util::Axis::y };
    util::Front m_baseFront{ util::Front::z };
    glm::vec3 m_baseAdjust{ 0.f };
    glm::vec3 m_offset{ 0.f };

    physics::Category m_category{ physics::Category::none };
    uint32_t m_collisionMask{ UINT_MAX };

    physics::ShapeType m_type{ physics::ShapeType::none };

    bool m_placeable : 1 { true };
};

struct PhysicsDefinition {
    bool m_update{ false };
    BodyDefinition m_body;
    ShapeDefinition m_shape;
};
