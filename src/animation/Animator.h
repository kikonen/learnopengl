#pragma once

#include <span>

#include <glm/glm.hpp>

#include "ki/size.h"

struct UpdateContext;

namespace animation {
    struct RigContainer;

    class Animator {
    public:
        // Update single palette with clipIndex animation clip
        // @return true if palette was changed
        bool animate(
            const UpdateContext& ctx,
            const animation::RigContainer& rig,
            const glm::mat4& meshRigTransform,
            const glm::mat4& inverseMeshRigTransform,
            const glm::mat4& animationBaseTransform,
            std::span<glm::mat4>& bonePalette,
            std::span<glm::mat4>& socketPalette,
            uint16_t clipIndex,
            double animationStartTime,
            double currentTime);
    };
}
