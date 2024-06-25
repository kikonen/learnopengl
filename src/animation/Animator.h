#pragma once

#include <span>

#include <glm/glm.hpp>

#include "ki/size.h"

struct UpdateContext;

namespace animation {
    struct RigContainer;

    class Animator {
    public:
        // Update single palette with animationIndex animation
        // @return true if palette was changed
        bool animate(
            const UpdateContext& ctx,
            const animation::RigContainer& rig,
            const glm::mat4& meshBaseTransform,
            const glm::mat4& inverseMeshBaseTransform,
            const glm::mat4& animationBaseTransform,
            std::span<glm::mat4>& palette,
            std::span<glm::mat4>& sockets,
            uint16_t animationIndex,
            double animationStartTime,
            double currentTime);
    };
}
