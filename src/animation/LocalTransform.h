#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace animation
{
    struct LocalTransform {
        glm::vec3 m_translate;
        glm::quat m_rotation;
        glm::vec3 m_scale;
    };
};
