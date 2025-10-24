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
#include "RigNodeChannel.h"
#include "Joint.h"

namespace {
    static const glm::mat4 ID_MAT{ 1.f };

   // @return interpolated transform matrix
   glm::mat4 interpolate(
       const animation::RigNodeChannel* channel,
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
       const animation::RigNodeChannel* channelA,
       const animation::RigNodeChannel* channelB,
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
        std::span<glm::mat4> rigNodeTransforms,
        std::span<glm::mat4> jointPalette,
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

        // STEP 1: update RigNode transforms
        // TODO KI stop at "max needed nodeIndex"
        for (size_t nodeIndex = 0; nodeIndex < rig.m_nodes.size(); nodeIndex++)
        {
            const auto& rigNode = rig.m_nodes[nodeIndex];
            const auto& parentTransform = nodeIndex > 0 ? rigNodeTransforms[rigNode.m_parentIndex] : ID_MAT;

            const bool accept = rigNode.m_jointRequired || rigNode.m_socketRequired;

            // NOTE KI skip joints not affecting animation/sockets
            if (!accept) continue;

            if (rigNode.m_jointRequired) {
                const auto* channel = animation.findByNodeIndex(rigNode.m_index);

                const glm::mat4& nodeTransform = channel
                    ? interpolate(channel, animationTimeTicks, firstFrame, lastFrame, clip.m_single)
                    : rigNode.m_transform;

                rigNodeTransforms[rigNode.m_index] = parentTransform * nodeTransform;
            }
            else {
                if (rigNode.m_socketRequired) {
                    rigNodeTransforms[rigNode.m_index] = parentTransform * rigNode.m_transform;
                }
            }
        }

        // STEP 2: update Joint Palette
        for (const auto& joint : rig.m_jointContainer.m_joints)
        {
            const auto& globalTransform = rigNodeTransforms[joint.m_nodeIndex];
            // NOTE KI m_offsetMatrix so that vertex is first converted to local space of joint
            jointPalette[joint.m_index] = inverseMeshRigTransform * globalTransform * joint.m_offsetMatrix;
        }

        // STEP 3: update Socket Palette
        for (const auto& socket : rig.m_sockets)
        {
            // NOTE KI see AnimationSystem::registerInstance()
            socketPalette[socket.m_index] = socket.calculateGlobalTransform(
                rigNodeTransforms[socket.m_nodeIndex]);
        }

        return true;
    }

    bool Animator::animateBlended(
        const animation::RigContainer& rig,
        const glm::mat4& inverseMeshRigTransform,
        std::span<glm::mat4> rigNodeTransforms,
        std::span<glm::mat4> jointPalette,
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

        // STEP 1: update RigNode transforms
        // TODO KI stop at "max needed nodeIndex"
        for (size_t nodeIndex = 0; nodeIndex < rig.m_nodes.size(); nodeIndex++)
        {
            const auto& rigNode = rig.m_nodes[nodeIndex];
            const auto& parentTransform = nodeIndex > 0 ? rigNodeTransforms[rigNode.m_parentIndex] : ID_MAT;

            bool accept = rigNode.m_jointRequired || rigNode.m_socketRequired;

            // NOTE KI skip joints not affecting animation/sockets
            if (!accept) continue;

            if (rigNode.m_jointRequired) {
                const auto* channelA = animationA->findByNodeIndex(rigNode.m_index);
                const auto* channelB = animationB->findByNodeIndex(rigNode.m_index);

                glm::mat4 nodeTransform;
                if (channelA && channelB) {
                    nodeTransform = interpolateBlended(
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
                    nodeTransform = interpolate(channelA, animationTimeTicksA, clipA->m_firstFrame, clipA->m_lastFrame, clipA->m_single);
                }
                else if (channelB) {
                    nodeTransform = interpolate(channelB, animationTimeTicksB, clipB->m_firstFrame, clipB->m_lastFrame, clipB->m_single);
                }
                else {
                    nodeTransform = rigNode.m_transform;
                }

                rigNodeTransforms[rigNode.m_index] = parentTransform * nodeTransform;
            }
            else {
                if (rigNode.m_socketRequired) {
                    rigNodeTransforms[rigNode.m_index] = parentTransform * rigNode.m_transform;
                }
            }
        }

        //// STEP 2: update Joint Palette
        //for (size_t nodeIndex = 0; nodeIndex < rig.m_nodes.size(); nodeIndex++)
        //{
        //    const auto& rigNode = rig.m_nodes[nodeIndex];
        //    const auto* joint = rig.m_jointContainer.getInfo(rigNode.m_jointIndex);

        //    if (joint) {
        //        const auto& globalTransform = rigNodeTransforms[rigNode.m_index];
        //        // NOTE KI m_offsetMatrix so that vertex is first converted to local space of joint
        //        jointPalette[joint->m_index] = inverseMeshRigTransform * globalTransform * joint->m_offsetMatrix;
        //    }
        //}

        // STEP 2: update Joint Palette
        for (const auto& joint : rig.m_jointContainer.m_joints)
        {
            const auto& globalTransform = rigNodeTransforms[joint.m_nodeIndex];
            // NOTE KI m_offsetMatrix so that vertex is first converted to local space of joint
            jointPalette[joint.m_index] = inverseMeshRigTransform * globalTransform * joint.m_offsetMatrix;
        }

        // STEP 3: update Socket Palette
        for (const auto& socket : rig.m_sockets) {
            // NOTE KI see AnimationSystem::registerInstance()
            socketPalette[socket.m_index] = socket.calculateGlobalTransform(
                rigNodeTransforms[socket.m_nodeIndex]);
        }

        return true;
    }
}
