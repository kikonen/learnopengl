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
#include "BoneInfo.h"

namespace {
   static const glm::mat4 ID_MAT{ 1.f };
}

namespace animation {
    bool Animator::animate(
        const UpdateContext& ctx,
        const animation::RigContainer& rig,
        const glm::mat4& meshRigTransform,
        const glm::mat4& inverseMeshRigTransform,
        const glm::mat4& animationRigTransform,
        std::span<glm::mat4>& bonePalette,
        std::span<glm::mat4>& socketPalette,
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

        constexpr size_t MAX_JOINTS = 200;
        glm::mat4 parentTransforms[MAX_JOINTS + 1];

        // NOTE KI cancel out modelMesh->m_transform set in AssimpLoader for mesh
        // => animation joint hierarchy includes equivalent transforms (presumeably)
        // NOTE KI order MATTERS when applying inverse
        //parentTransforms[0] = inverseMeshRigTransform * animationRigTransform;
        parentTransforms[0] = animationRigTransform;
        //parentTransforms[0] = ID_MAT;
        //parentTransforms[0] *= glm::translate(
        //    ID_MAT,
        //    glm::vec3{ 0.f, 24.5f, 0.f });

        size_t hitCount = 0;
        size_t missCount = 0;

        //std::vector<std::string> hitMiss;
        //bool boneFound = false;

        //for (auto& mat : bonePalette) {
        //    mat = inverseMeshRigTransform * animationRigTransform;
        //    mat = ID_MAT;
        //}

        bool foundRoot = false;
        for (size_t jointIndex = 0; jointIndex < rig.m_joints.size(); jointIndex++)
        {
            const auto& rigJoint = rig.m_joints[jointIndex];

            bool accept = rigJoint.m_boneRequired || rigJoint.m_socketRequired;

            // NOTE KI skip joints not affecting animation/sockets
            if (!accept) continue;

            //if (hitCount >= 5) break;

            if (rigJoint.m_boneRequired) {
                if (rigJoint.m_index >= MAX_JOINTS) throw "too many joints";
                //auto* bone = rig.m_boneContainer.findByJoingIndex(rigJoint.m_index);
                const auto* bone = rig.m_boneContainer.getInfo(rigJoint.m_boneIndex);

                //if (bone) {
                //    if (!boneFound) {
                //        //parentTransforms[rigJoint.m_parentIndex + 1] = glm::inverse(rigJoint.m_transform);
                //        parentTransforms[rigJoint.m_parentIndex + 1] = ID_MAT;
                //    }
                //    boneFound |= true;
                //}
                //else {
                //    if (!boneFound) continue;
                //}

                const auto* channel = animation->findByJointIndex(rigJoint.m_index);

                const glm::mat4& jointTransform = channel
                    ? channel->interpolate(animationTimeTicks)
                    : rigJoint.m_transform;

                //if (rigJoint.m_name == std::string{ "SkeletonKnight_" })
                //{
                //    jointTransform = ID_MAT;
                //}

                parentTransforms[rigJoint.m_index + 1] = parentTransforms[rigJoint.m_parentIndex + 1] * jointTransform;
                const auto& globalTransform = parentTransforms[rigJoint.m_index + 1];

                //KI_INFO_OUT(fmt::format(
                //    "{},{} - {}\npare: {}\njoin: {}\nglob: {}\noffs: {}\npale: {}\n",
                //    rigJoint.m_parentIndex,
                //    rigJoint.m_index,
                //    rigJoint.m_name,
                //    parentTransforms[rigJoint.m_parentIndex + 1],
                //    jointTransform,
                //    globalTransform,
                //    bone ? bone->m_offsetMatrix : glm::mat4{ 0.f },
                //    bone ? globalTransform * bone->m_offsetMatrix : glm::mat4{ 0.f }));

                //auto* bone = rig.m_boneContainer.findByJointIndex(rigJoint.m_index);
                if (bone) {
                    //hitMiss.push_back(fmt::format("[+{}.{}.{}]",
                    //    rigJoint.m_parentIndex, rigJoint.m_index, rigJoint.m_name));

                    hitCount++;

                    //if (channel) {
                    //    channel->interpolate(animationTimeTicks);
                    //}

                    // NOTE KI m_offsetMatrix so that vertex is first converted to local space of bone
                    bonePalette[bone->m_index] = inverseMeshRigTransform * globalTransform * bone->m_offsetMatrix;
                }
                else {
                    //hitMiss.push_back(fmt::format("[-{}.{}.{}]",
                    //    rigJoint.m_parentIndex, rigJoint.m_index, rigJoint.m_name));
                    //missCount++;
                }
            }
            else {
                if (rigJoint.m_socketRequired) {
                    parentTransforms[rigJoint.m_index + 1] = parentTransforms[rigJoint.m_parentIndex + 1] * rigJoint.m_transform;
                }
            }
        }

        for (const auto& socket : rig.m_sockets) {
            socketPalette[socket.m_index] = parentTransforms[socket.m_jointIndex + 1] *
                socket.m_transform;
        }

        //KI_INFO_OUT(fmt::format("ANIM: joints={}, bones={}, hit={}, miss={}, graph={}",
        //    rig.m_joints.size(), rig.m_boneContainer.m_boneInfos.size(), hitCount, missCount,
        //    util::join(hitMiss, "")));

        return true;
    }
}
