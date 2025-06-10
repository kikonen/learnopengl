#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct CreateState
{
    glm::vec3 m_position{ 0.f };
    glm::vec3 m_scale{ 1.f };
    glm::quat m_rotation{ 1.f, 0.f, 0.f, 0.f };
};
