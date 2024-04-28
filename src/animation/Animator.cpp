#include "Animator.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "RigContainer.h"
#include "Animation.h"
#include "BoneChannel.h"
#include "MatrixPalette.h"

namespace animation {
    void Animator::animate(
        const UpdateContext& ctx,
        const animation::RigContainer& rig,
        uint16_t animationIndex,
        MatrixPalette& palette,
        const Node& node)
    {
        auto* animation = rig.m_animations[animationIndex].get();

        for (auto& channel : animation->m_channels) {
            // 0) find Node/Bone matching channel
            // 1) find curr/prev key frame for pos/rotation/scale in channel
            // 2) interpolate between curr/prev

            auto* node = rig.getNode(channel.m_nodeId);
        }
    }
}
