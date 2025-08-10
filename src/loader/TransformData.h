#pragma once

#include <string>

#include "util/Transform.h"

#include <glm/glm.hpp>

namespace loader {
    struct TransformData {
        glm::vec3 position{ 0.f };
        glm::vec3 rotation{ 0.f };
        glm::vec3 scale{ 1.f };

        util::Transform toTransform() const noexcept
        {
            return {
                position,
                util::degreesToQuat(rotation),
                scale,
            };
        }
    };
}
