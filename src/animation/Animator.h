#pragma once

#include <span>

#include <glm/glm.hpp>

#include "ki/size.h"

namespace animation {
    struct RigContainer;
    struct LocalTransform;

    class Animator {
    public:
        // Update single palette with clipIndex animation clip
        // @return true if palette was changed
        bool animate(
            const animation::RigContainer& rig,
            const glm::mat4& meshRigTransform,
            const glm::mat4& inverseMeshRigTransform,
            const glm::mat4& animationBaseTransform,
            std::span<glm::mat4> bonePalette,
            std::span<glm::mat4> socketPalette,
            uint16_t clipIndex,
            double animationStartTime,
            float speed,
            double currentTime,
            bool forceFirstFrame);

        // Update single palette with clipIndex animation clip
        // @return true if palette was changed
        bool animateBlended(
            const animation::RigContainer& rig,
            const glm::mat4& meshRigTransform,
            const glm::mat4& inverseMeshRigTransform,
            const glm::mat4& animationBaseTransform,
            std::span<glm::mat4> bonePalette,
            std::span<glm::mat4> socketPalette,
            uint16_t clipIndexA,
            double animationStartTimeA,
            float speedA,
            uint16_t clipIndexB,
            double animationStartTimeB,
            float speedB,
            float blendFactor,
            double currentTime,
            bool forceFirstFrame);
    };
}
