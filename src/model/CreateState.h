#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct CreateState
{
    glm::vec3 m_position{ 0.f };
    glm::vec3 m_scale{ 1.f };
    glm::quat m_rotation{ 1.f, 0.f, 0.f, 0.f };

    ki::tag_id m_tagId{ 0 };

    float m_tilingX{ 1.f };
    float m_tilingY{ 1.f };
};
