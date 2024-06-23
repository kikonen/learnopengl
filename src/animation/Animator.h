#pragma once

#include <span>

#include <glm/glm.hpp>

#include "ki/size.h"

struct UpdateContext;

namespace animation {
    struct RigContainer;
    struct BoneTransform;

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
            std::span<animation::BoneTransform>& palette,
            uint16_t animationIndex,
            double animationStartTime,
            double currentTime);
    };
}
