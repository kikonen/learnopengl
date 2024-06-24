#include "Animator.h"

#include <iostream>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <fmt/format.h>

#include "engine/UpdateContext.h"

#include "util/glm_util.h"
#include "util/glm_format.h"
#include "util/Util.h"

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
        const glm::mat4& meshBaseTransform,
        const glm::mat4& inverseMeshBaseTransform,
        const glm::mat4& animationBaseTransform,
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

        constexpr size_t MAX_NODES = 200;
        glm::mat4 parentTransforms[MAX_NODES + 1];

        // NOTE KI cancel out modelMesh->m_transform set in AssimpLoader for mesh
        // => animation node hierarchy includes equivalent transforms (presumeably)
        // NOTE KI order MATTERS when applying inverse
        parentTransforms[0] = inverseMeshBaseTransform * animationBaseTransform;
        //parentTransforms[0] = animationBaseTransform;
        //parentTransforms[0] = glm::mat4(1.f);
        //parentTransforms[0] *= glm::translate(
        //    glm::mat4(1.f),
        //    glm::vec3{ 0.f, 24.5f, 0.f });

        size_t hitCount = 0;
        size_t missCount = 0;

        std::vector<std::string> hitMiss;
        //bool boneFound = false;

        for (size_t nodeIndex = 0; nodeIndex < rig.m_nodes.size(); nodeIndex++)
        {
            const auto& rigNode = rig.m_nodes[nodeIndex];

            // NOTE KI skip nodes not affecting animation
            if (!rigNode.m_boneRequired) continue;

            //if (hitCount >= 1) break;

            if (rigNode.m_index >= MAX_NODES) throw "too many bones";
            //auto* bone = rig.m_boneContainer.findByNodeIndex(rigNode.m_index);
            const auto* bone = rig.m_boneContainer.getNode(rigNode.m_boneIndex);

            //if (bone) {
            //    if (!boneFound) {
            //        //parentTransforms[rigNode.m_parentIndex + 1] = glm::inverse(rigNode.m_localTransform);
            //        parentTransforms[rigNode.m_parentIndex + 1] = glm::mat4(1.f);
            //    }
            //    boneFound |= true;
            //}
            //else {
            //    if (!boneFound) continue;
            //}

            const auto* channel = animation->findByNodeIndex(rigNode.m_index);
            const glm::mat4& nodeTransform = channel
                ? channel->interpolate(animationTimeTicks)
                : rigNode.m_localTransform;

            parentTransforms[rigNode.m_index + 1] = parentTransforms[rigNode.m_parentIndex + 1] * nodeTransform;
            const auto& globalTransform = parentTransforms[rigNode.m_index + 1];

            //KI_INFO_OUT(fmt::format(
            //    "{},{} - {}\npare: {}\nnode: {}\nglob: {}\noffs: {}\npale: {}\n",
            //    rigNode.m_parentIndex,
            //    rigNode.m_index,
            //    rigNode.m_name,
            //    parentTransforms[rigNode.m_parentIndex + 1],
            //    nodeTransform,
            //    globalTransform,
            //    bone ? bone->m_offsetMatrix : glm::mat4{ 0.f },
            //    bone ? globalTransform * bone->m_offsetMatrix : glm::mat4{ 0.f }));

            //auto* bone = rig.m_boneContainer.findByNodeIndex(rigNode.m_index);
            if (bone) {
                //hitMiss.push_back(fmt::format("[+{}.{}.{}]",
                //    rigNode.m_parentIndex, rigNode.m_index, rigNode.m_name));

                //hitCount++;

                //if (channel) {
                //    channel->interpolate(animationTimeTicks);
                //}

                // NOTE KI m_offsetMatrix so that vertex is first converted to local space of bone
                palette[bone->m_index].m_transform = globalTransform * bone->m_offsetMatrix;

            }
            else {
                //hitMiss.push_back(fmt::format("[-{}.{}.{}]",
                //    rigNode.m_parentIndex, rigNode.m_index, rigNode.m_name));
                //missCount++;
            }
        }

        //KI_INFO_OUT(fmt::format("ANIM: nodes={}, bones={}, hit={}, miss={}, graph={}",
        //    rig.m_nodes.size(), rig.m_boneContainer.m_boneInfos.size(), hitCount, missCount,
        //    util::join(hitMiss, "")));

        return true;
    }
}
