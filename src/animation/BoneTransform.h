#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

namespace animation {
    struct BoneTransform {
        glm::mat4 m_offsetMatrix;
    };
}
