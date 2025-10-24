#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

namespace animation {
    struct JointTransform {
        JointTransform()
            : m_transform{ 1.f }
        {}

        JointTransform(const glm::mat4& transform)
            : m_transform{ transform }
        {}

        glm::mat4 m_transform;
    };
}
