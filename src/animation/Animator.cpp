#include "Animator.h"

#include <iostream>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <fmt/format.h>

#include "engine/UpdateContext.h"

#include "util/glm_util.h"
#include "util/glm_format.h"
#include "util/util.h"
#include "util/Transform.h"

#include "RigContainer.h"
#include "Animation.h"
#include "BoneChannel.h"
#include "BoneInfo.h"

namespace {
    constexpr size_t MAX_JOINTS = 200;

    static const glm::mat4 ID_MAT{ 1.f };

   // @return interpolated transform matrix
   glm::mat4 interpolate(
       const animation::BoneChannel* channel,
       float animationTimeTicks,
       uint16_t firstFrame,
       uint16_t lastFrame,
       bool single)
   {
       if (single) {
           firstFrame = 0;
       }

       util::Transform transform;
       channel->interpolate(animationTimeTicks, firstFrame, lastFrame, single, transform);
       return transform.toMatrix();
   }

   glm::mat4 combineBlended(
       const util::Transform& transformA,
       const util::Transform& transformB,
       float blendFactor)
   {
       //static const glm::mat4 ID_MAT{ 1.f };
       glm::mat4 s_translateMatrix{ 1.f };
       glm::mat4 s_scaleMatrix{ 1.f };

       {
           const auto blendedTranslate =
               (1.f - blendFactor) * transformA.m_position +
               transformB.m_position * blendFactor;

           s_translateMatrix[3].x = blendedTranslate.x;
           s_translateMatrix[3].y = blendedTranslate.y;
           s_translateMatrix[3].z = blendedTranslate.z;
       }
       {
           const auto blendedScale =
               (1.f - blendFactor) * transformA.m_scale +
               transformB.m_scale * blendFactor;

           s_scaleMatrix[0].x = blendedScale.x;
           s_scaleMatrix[1].y = blendedScale.y;
           s_scaleMatrix[2].z = blendedScale.z;
       }

       glm::mat4 rotateMatrix;
       {
           const auto blendedRotation = glm::slerp(transformA.m_rotation, transformB.m_rotation, blendFactor);
           rotateMatrix = glm::toMat4(blendedRotation);
       }

       return s_translateMatrix * rotateMatrix * s_scaleMatrix;
   }

   // @return interpolated transform matrix
   glm::mat4 interpolateBlended(
       const animation::BoneChannel* channelA,
       const animation::BoneChannel* channelB,
       float blendFactor,
       float animationTimeTicksA,
       uint16_t firstFrameA,
       uint16_t lastFrameA,
       bool singleA,
       float animationTimeTicksB,
       uint16_t firstFrameB,
       uint16_t lastFrameB,
       bool singleB)
   {
       if (singleA) {
           firstFrameA = 0;
       }
       if (singleB) {
           firstFrameB = 0;
       }

       util::Transform transformA;
       util::Transform transformB;

       channelA->interpolate(animationTimeTicksA, firstFrameA, lastFrameA, singleA, transformA);
       channelB->interpolate(animationTimeTicksB, firstFrameB, lastFrameB, singleB, transformB);

       return combineBlended(transformA, transformB, blendFactor);
   }
}

namespace animation {
    bool Animator::animate(
        const animation::RigContainer& rig,
        const glm::mat4& inverseMeshRigTransform,
        std::span<glm::mat4> bonePalette,
        std::span<glm::mat4> socketPalette,
        uint16_t clipIndex,
        double animationStartTime,
        float speed,
        double currentTime,
        bool forceFirstFrame)
    {
        if (animationStartTime < 0) return false;
        const auto& clipContainer = rig.m_clipContainer;
        if (clipIndex < 0 || clipIndex >= clipContainer.m_clips.size()) return false;

        const auto& clip = clipContainer.m_clips[clipIndex];
        if (clip.m_animationIndex < 0) return false;
        if (clip.m_duration <= 0.f) return false;

        const auto& animation = *clipContainer.m_animations[clip.m_animationIndex];
        const auto firstFrame = clip.m_firstFrame;
        const auto lastFrame = clip.m_lastFrame;

        float animationTimeTicks = clipContainer.getAnimationTimeTicks(
            clipIndex,
            animationStartTime,
            forceFirstFrame ? animationStartTime : currentTime,
            speed);

        glm::mat4 globalTransforms[MAX_JOINTS + 1];

        // NOTE KI cancel out modelMesh->m_transform set in AssimpLoader for mesh
        // => animation joint hierarchy includes equivalent transforms (presumeably)
        // NOTE KI order MATTERS when applying inverse
        globalTransforms[0] = glm::mat4{ 1.f };

        for (size_t jointIndex = 0; jointIndex < rig.m_joints.size(); jointIndex++)
        {
            const auto& rigJoint = rig.m_joints[jointIndex];

            bool accept = rigJoint.m_boneRequired || rigJoint.m_socketRequired;

            // NOTE KI skip joints not affecting animation/sockets
            if (!accept) continue;

            if (rigJoint.m_boneRequired) {
                if (rigJoint.m_index >= MAX_JOINTS) throw "too many joints";
                const auto* bone = rig.m_boneContainer.getInfo(rigJoint.m_boneIndex);

                const auto* channel = animation.findByJointIndex(rigJoint.m_index);

                const glm::mat4& jointTransform = channel
                    ? interpolate(channel, animationTimeTicks, firstFrame, lastFrame, clip.m_single)
                    : rigJoint.m_transform;

                globalTransforms[rigJoint.m_index + 1] = globalTransforms[rigJoint.m_parentIndex + 1] * jointTransform;
                const auto& globalTransform = globalTransforms[rigJoint.m_index + 1];

                if (bone) {
                    // NOTE KI m_offsetMatrix so that vertex is first converted to local space of bone
                    bonePalette[bone->m_index] = inverseMeshRigTransform * globalTransform * bone->m_offsetMatrix;
                }
            }
            else {
                if (rigJoint.m_socketRequired) {
                    globalTransforms[rigJoint.m_index + 1] = globalTransforms[rigJoint.m_parentIndex + 1] * rigJoint.m_transform;
                }
            }
        }

        for (const auto& socket : rig.m_sockets) {
            // NOTE KI see AnimationSystem::registerInstance()
            socketPalette[socket.m_index] = socket.calculateGlobalTransform(
                globalTransforms[socket.m_jointIndex + 1]);
        }

        return true;
    }

    bool Animator::animateBlended(
        const animation::RigContainer& rig,
        const glm::mat4& inverseMeshRigTransform,
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
        bool forceFirstFrame)
    {
        if (animationStartTimeA < 0) return false;
        const auto& clipContainer = rig.m_clipContainer;

        const Clip* clipA{ nullptr };
        const Animation* animationA{ nullptr };

        const Clip* clipB{ nullptr };
        const Animation* animationB{ nullptr };

        {
            if (clipIndexA < 0 || clipIndexA >= clipContainer.m_clips.size()) return false;

            clipA = &clipContainer.m_clips[clipIndexA];
            if (clipA->m_animationIndex < 0) return false;
            if (clipA->m_duration <= 0.f) return false;
            animationA = clipContainer.m_animations[clipA->m_animationIndex].get();
        }
        {
            if (clipIndexB < 0 || clipIndexB >= clipContainer.m_clips.size()) return false;

            clipB = &clipContainer.m_clips[clipIndexB];
            if (clipB->m_animationIndex < 0) return false;
            if (clipB->m_duration <= 0.f) return false;
            animationB = clipContainer.m_animations[clipB->m_animationIndex].get();
        }

        if (!animationA || !animationB) return false;

        float animationTimeTicksA = clipContainer.getAnimationTimeTicks(
            clipIndexA,
            animationStartTimeA,
            forceFirstFrame ? animationStartTimeA : currentTime,
            speedA);

        float animationTimeTicksB = clipContainer.getAnimationTimeTicks(
            clipIndexB,
            animationStartTimeB,
            forceFirstFrame ? animationStartTimeB : currentTime,
            speedB);

        glm::mat4 globalTransforms[MAX_JOINTS + 1];

        // NOTE KI cancel out modelMesh->m_transform set in AssimpLoader for mesh
        // => animation joint hierarchy includes equivalent transforms (presumeably)
        // NOTE KI order MATTERS when applying inverse
        globalTransforms[0] = glm::mat4{ 1.f };

        for (size_t jointIndex = 0; jointIndex < rig.m_joints.size(); jointIndex++)
        {
            const auto& rigJoint = rig.m_joints[jointIndex];

            bool accept = rigJoint.m_boneRequired || rigJoint.m_socketRequired;

            // NOTE KI skip joints not affecting animation/sockets
            if (!accept) continue;

            if (rigJoint.m_boneRequired) {
                if (rigJoint.m_index >= MAX_JOINTS) throw "too many joints";
                const auto* bone = rig.m_boneContainer.getInfo(rigJoint.m_boneIndex);

                const auto* channelA = animationA->findByJointIndex(rigJoint.m_index);
                const auto* channelB = animationB->findByJointIndex(rigJoint.m_index);

                glm::mat4 jointTransform;
                if (channelA && channelB) {
                    jointTransform = interpolateBlended(
                        channelA,
                        channelB,
                        blendFactor,
                        animationTimeTicksA,
                        clipA->m_firstFrame,
                        clipA->m_lastFrame,
                        clipA->m_single,
                        animationTimeTicksB,
                        clipB->m_firstFrame,
                        clipB->m_lastFrame,
                        clipB->m_single);
                }
                else if (channelA) {
                    jointTransform = interpolate(channelA, animationTimeTicksA, clipA->m_firstFrame, clipA->m_lastFrame, clipA->m_single);
                }
                else if (channelB) {
                    jointTransform = interpolate(channelB, animationTimeTicksB, clipB->m_firstFrame, clipB->m_lastFrame, clipB->m_single);
                }
                else {
                    jointTransform = rigJoint.m_transform;
                }

                globalTransforms[rigJoint.m_index + 1] = globalTransforms[rigJoint.m_parentIndex + 1] * jointTransform;

                if (bone) {
                    // NOTE KI m_offsetMatrix so that vertex is first converted to local space of bone
                    const auto& globalTransform = globalTransforms[rigJoint.m_index + 1];
                    bonePalette[bone->m_index] = inverseMeshRigTransform * globalTransform * bone->m_offsetMatrix;
                }
            }
            else {
                if (rigJoint.m_socketRequired) {
                    globalTransforms[rigJoint.m_index + 1] = globalTransforms[rigJoint.m_parentIndex + 1] * rigJoint.m_transform;
                }
            }
        }

        for (const auto& socket : rig.m_sockets) {
            socketPalette[socket.m_index] = socket.calculateGlobalTransform(
                globalTransforms[socket.m_jointIndex + 1]);
        }

        return true;
    }
}
