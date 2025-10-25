#pragma once

#include <span>

#include <glm/glm.hpp>

#include "ki/size.h"

namespace util
{
    struct Transform;
}

namespace animation {
    struct RigContainer;

    class Animator {
    public:
        // Update single palette with clipIndex animation clip
        // @return true if palette was changed
        bool animate(
            const animation::RigContainer& rig,
            std::span<glm::mat4> rigNodeTransforms,
            uint16_t clipIndex,
            double animationStartTime,
            float speed,
            double currentTime,
            bool forceFirstFrame);

        // Update single palette with clipIndex animation clip
        // @return true if palette was changed
        bool animateBlended(
            const animation::RigContainer& rig,
            std::span<glm::mat4> rigNodeTransforms,
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
