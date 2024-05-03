#include "Animator.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "engine/UpdateContext.h"

#include "util/glm_util.h"

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
        float animationStartTime,
        float currentTime)
    {
        if (animationStartTime < 0) return false;

        auto* animation = rig.m_animations[animationIndex].get();

        auto quat = util::degreesToQuat(glm::vec3{ 0.f, .2f, 0.f });
        auto rotationMatrix = glm::toMat4(quat);

        for (auto i = 0; i < rig.m_boneContainer.size(); i++) {
            palette[i].m_transform = palette[i].m_transform * rotationMatrix;
        }

        for (auto& channel : animation->m_channels) {
            // 0) find Node/Bone matching channel
            // 1) find curr/prev key frame for pos/rotation/scale in channel
            // 2) interpolate between curr/prev

            auto* node = rig.getNode(channel.m_nodeId);
        }

        return true;
    }
}
