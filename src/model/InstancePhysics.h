#pragma once

#include <glm/glm.hpp>


namespace model
{
    struct InstancePhysics {
        float m_velocity;
        glm::vec3 m_axis;
        float m_angularRotation;
    };
}
