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
            std::span<animation::BoneTransform>& palette,
            uint16_t animationIndex,
            double animationStartTime,
            double currentTime);

        //void animateHierarchy(
        //    const animation::RigContainer& rig,
        //    std::span<animation::BoneTransform>& palette,
        //    float animationTimeTicks,
        //    int16_t nodeIndex,
        //    const glm::mat4& parentTransform);

        //glm::mat4 m_globalInverseTransform;
    };
}
