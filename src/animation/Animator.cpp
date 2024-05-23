#include "Animator.h"

#include <iostream>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <fmt/format.h>

#include "engine/UpdateContext.h"

#include "util/glm_util.h"

#include "RigContainer.h"
#include "Animation.h"
#include "BoneChannel.h"
#include "BoneTransform.h"
#include "BoneInfo.h"

namespace {
   static const glm::mat4 ID_MAT{ 1.f };
}

namespace animation {
    bool Animator::animate(
        const UpdateContext& ctx,
        const animation::RigContainer& rig,
        const glm::mat4& meshTransform,
        std::span<animation::BoneTransform>& palette,
        uint16_t animationIndex,
        double animationStartTime,
        double currentTime)
    {
        if (animationStartTime < 0) return false;
        if (animationIndex < 0 || animationIndex >= rig.m_animations.size()) return false;

        auto* animation = rig.m_animations[animationIndex].get();

        //auto quat = util::degreesToQuat(glm::vec3{ 0.f, .2f, 0.f });
        //auto rotationMatrix = glm::toMat4(quat);

        //{
        //    for (auto i = 0; i < rig.m_boneContainer.size(); i++) {
        //        //palette[i].m_transform = palette[i].m_transform * rotationMatrix;
        //        palette[i].m_transform = ID_MAT;
        //    }
        //}

        float animationTimeTicks;
        {
            float animationTimeSecs = (float)(currentTime - animationStartTime);
            float ticksPerSecond = animation->m_ticksPerSecond != 0.f ? animation->m_ticksPerSecond : 25.f;
            float timeInTicks = animationTimeSecs * ticksPerSecond;
            animationTimeTicks = fmod(timeInTicks, animation->m_duration);

            //std::cout << fmt::format(
            //    "time={}, secs={}, ticksSec={}, ticks={}, duration={}\n",
            //    currentTime, animationTimeSecs, ticksPerSecond, timeInTicks, animation->m_duration);
        }


        //const auto globalInverseTransform = glm::inverse(rig.m_nodes[0].m_globalTransform);
        const auto globalInverseTransform = glm::inverse(meshTransform);

        //for (int16_t nodeIndex = 0; nodeIndex < rig.m_nodes.size(); nodeIndex++) {
        //}

        constexpr size_t MAX_NODES = 200;
        glm::mat4 parentTransforms[MAX_NODES + 1];
        parentTransforms[0] = ID_MAT;

        for (const auto& rigNode : rig.m_nodes) {
            if (rigNode.m_index >= MAX_NODES) throw "too many bones";
            const auto* channel = animation->findByNodeIndex(rigNode.m_index);
            const glm::mat4& nodeTransform = channel
                ? channel->interpolate(animationTimeTicks)
                //: rigNode.m_localTransform;
                : ID_MAT;

            parentTransforms[rigNode.m_index + 1] = parentTransforms[rigNode.m_parentIndex + 1] * nodeTransform;
            const auto& globalTransform = parentTransforms[rigNode.m_index + 1];

            auto* bone = rig.m_boneContainer.findByNodeIndex(rigNode.m_index);
            if (bone) {
                //palette[bone->m_index] = globalTransform * bone->m_offsetMatrix;
                palette[bone->m_index] = globalInverseTransform * globalTransform * bone->m_offsetMatrix;
                //palette[bone->m_index] = rigNode.m_globalInvTransform * nodeTransform * bone->m_offsetMatrix;
            }
        }

        return true;
    }

    //void Animator::animateHierarchy(
    //    const animation::RigContainer& rig,
    //    std::span<animation::BoneTransform>& palette,
    //    float animationTimeTicks,
    //    int16_t nodeIndex,
    //    const glm::mat4& parentTransform)
    //{
    //}
}
