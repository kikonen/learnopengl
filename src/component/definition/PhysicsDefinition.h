#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "physics/Category.h"
#include "physics/size.h"

// @see physics/Body.h
struct BodyDefinition {
    glm::vec3 m_size{ 1.f };

    glm::quat m_baseRotation{ 1.f, 0.f, 0.f, 0.f };

    glm::vec3 m_linearVelocity{ 0.f };
    glm::vec3 m_angularVelocity{ 0.f };

    glm::vec3 m_axis{ 0.f, 1.f, 0.f };

    float m_maxAngulerVelocity{ 100.f };
    float m_density{ 1.f };

    physics::BodyType m_type{ physics::BodyType::none };

    bool m_forceAxis : 1 { false };
    bool m_kinematic : 1 { false };
};

// @see physics/Geom.h
struct GeomDefinition {
    bool m_enabled{ false };

    glm::vec3 m_size{ 0.5f };

    glm::quat m_rotation{ 1.f, 0.f, 0.f, 0.f };
    glm::vec3 m_offset{ 0.f };

    uint32_t m_categoryMask{ UINT_MAX };
    uint32_t m_collisionMask{ UINT_MAX };

    physics::GeomType m_type{ physics::GeomType::none };

    bool m_placeable : 1 { true };
};

struct PhysicsDefinition {
    bool m_update{ false };
    BodyDefinition m_body;
    GeomDefinition m_geom;
};
