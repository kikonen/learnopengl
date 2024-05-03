#include "Animator.h"

#include "engine/UpdateContext.h"

#include "RigContainer.h"
#include "Animation.h"
#include "BoneChannel.h"
#include "BoneTransform.h"

namespace animation {
    bool Animator::animate(
        const UpdateContext& ctx,
        const animation::RigContainer& rig,
        std::span<animation::BoneTransform>& palette,
        uint16_t animationIndex,
        float animationStartTime)
    {
        if (animationStartTime < 0) return false;

        auto* animation = rig.m_animations[animationIndex].get();

        for (auto& channel : animation->m_channels) {
            // 0) find Node/Bone matching channel
            // 1) find curr/prev key frame for pos/rotation/scale in channel
            // 2) interpolate between curr/prev

            auto* node = rig.getNode(channel.m_nodeId);
        }
    }
}
