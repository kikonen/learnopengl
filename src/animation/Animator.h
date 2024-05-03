#pragma once

#include <span>

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
            float animationStartTime,
            float currentTime);
    };
}
