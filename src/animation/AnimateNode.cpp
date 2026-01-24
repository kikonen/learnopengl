#include "AnimateNode.h"

#include <limits>
#include <unordered_map>

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "mesh/Mesh.h"
#include "mesh/LodMesh.h"
#include "mesh/RegisteredRig.h"

#include "debug/DebugContext.h"

#include "animation/AnimationState.h"
#include "animation/Rig.h"
#include "animation/RigNode.h"
#include "animation/JointContainer.h"
#include "animation/Joint.h"
#include "animation/Animator.h"

#include "animation/RigNodeRegistry.h"
#include "animation/SocketRegistry.h"
#include "animation/JointRegistry.h"
#include "animation/RigSocket.h"

namespace {
    const glm::mat4 ID_MAT{ 1.f };
}

namespace animation
{
    AnimateNode::AnimateNode(
        RigNodeRegistry& rigNodeRegistry,
        JointRegistry& jointRegistry,
        SocketRegistry& socketRegistry,
        std::mutex& volumeLock,
        bool onceOnly)
        : m_rigNodeRegistry{ rigNodeRegistry },
        m_jointRegistry{ jointRegistry },
        m_socketRegistry{ socketRegistry },
        m_volumeLock{ volumeLock },
        m_onceOnly{ onceOnly }
    {}

    void AnimateNode::animate(
        const UpdateContext& ctx,
        AnimationState& state,
        model::Node* node)
    {
        const auto& dbg = debug::DebugContext::modify();
        const auto& anim = dbg.m_animation;

        if (anim.m_paused) return;

        auto& playA = state.m_current;
        auto& playB = state.m_next;
        {
            if (state.m_pending.m_active) {
                playB = state.m_pending;
                state.m_pending.m_active = false;
            }
            if (!playA.m_active && playB.m_active) {
                playA = playB;
                playB.m_active = false;
            }

            if (!playA.m_active && !anim.m_debugEnabled)
                return;
        }

        std::set<const Rig*> changedRigs;

        animateRigs(ctx, state, node, changedRigs);
        updateJointsAndSockets(node, changedRigs);
        if (!changedRigs.empty()) {
            updateAnimatedVolume(state, node);
            updateGroundOffset(state, node);
        }
    }

    void AnimateNode::animateRigs(
        const UpdateContext& ctx,
        AnimationState& state,
        model::Node* node,
        std::set<const Rig*>& changedRigs)
    {
        const auto& dbg = debug::DebugContext::modify();
        const auto& anim = dbg.m_animation;

        auto& playA = state.m_current;
        auto& playB = state.m_next;

        const auto& registeredRigs = node->getRegisteredRigs();

        for (const auto& registeredRig : registeredRigs) {
            auto rigNodeTransforms = m_rigNodeRegistry.modifyRange(registeredRig.m_rigRef);
            const auto* rig = registeredRig.m_rig;

            double currentTime = ctx.getClock().ts;

            float blendFactor = -1.f;

            if (anim.m_debugEnabled) {
                playA.m_clipIndex = anim.m_clipIndexA;
                playB.m_clipIndex = anim.m_clipIndexB;

                playA.m_speed = anim.m_speedA;
                playB.m_speed = anim.m_speedB;

                blendFactor = anim.m_blendFactor;

                if (anim.m_manualTime) {
                    currentTime = anim.m_currentTime;
                    playA.m_startTime = anim.m_startTimeA;
                    playB.m_startTime = anim.m_startTimeB;
                }
                else {
                    if (playA.m_startTime < 1000.f) {
                        playA.m_startTime = currentTime;
                    }
                    if (playB.m_startTime < 1000.f) {
                        playB.m_startTime = currentTime + 3.f;
                    }
                }

                if (!anim.m_blend) {
                    playB.m_clipIndex = -1;
                    playB.m_active = false;
                }
            }

            if (playB.m_active) {
                if (blendFactor < 0) {
                    if (playB.m_blendTime > 0) {
                        auto diff = currentTime - playB.m_startTime;
                        blendFactor = static_cast<float>(diff / playB.m_blendTime);
                    }
                    else {
                        blendFactor = 1.f;
                    }
                }

                blendFactor = std::max(std::min(blendFactor, 1.f), 0.f);

                // NOTE KI next is completely blended
                if (blendFactor >= 1.f) {
                    playA = playB;
                    playB.m_active = false;
                }
            }

            bool changed = false;
            Animator animator;
            if (!playB.m_active) {
                changed = animator.animate(
                    *rig,
                    rigNodeTransforms,
                    playA.m_clipIndex,
                    playA.m_startTime,
                    playA.m_speed,
                    currentTime,
                    anim.m_forceFirstFrame);
            }
            else {
                changed = animator.animateBlended(
                    *rig,
                    rigNodeTransforms,
                    playA.m_clipIndex,
                    playA.m_startTime,
                    playA.m_speed,
                    playB.m_clipIndex,
                    playB.m_startTime,
                    playB.m_speed,
                    blendFactor,
                    currentTime,
                    anim.m_forceFirstFrame);
            }

            if (m_onceOnly) {
                playA.m_active = false;
                playB.m_active = false;
            }

            if (changed) {
                m_rigNodeRegistry.markDirty(registeredRig.m_rigRef);
                changedRigs.insert(rig);
            }
        }
    }

    void AnimateNode::updateJointsAndSockets(
        model::Node* node,
        const std::set<const Rig*>& changedRigs)
    {
        const auto& registeredRigs = node->getRegisteredRigs();

        for (const auto& registeredRig : registeredRigs) {
            const auto* rig = registeredRig.m_rig;

            if (!changedRigs.contains(rig)) continue;

            const auto& rigNodeTransforms = m_rigNodeRegistry.getRange(registeredRig.m_rigRef);
            const auto& jointContainer = rig->getJointContainer();

            {
                auto jointPalette = m_jointRegistry.modifyRange(registeredRig.m_jointRef);
                auto socketPalette = m_socketRegistry.modifyRange(registeredRig.m_socketRef);

                // Update Joint Palette
                for (const auto& joint : jointContainer.m_joints)
                {
                    const auto& globalTransform = joint.m_nodeIndex >= 0 ? rigNodeTransforms[joint.m_nodeIndex] : ID_MAT;

                    // NOTE KI m_offsetMatrix so that vertex is first converted to local space of joint
                    jointPalette[joint.m_jointIndex] = globalTransform * joint.m_offsetMatrix;
                }

                // Update Socket Palette
                for (const auto& socket : rig->m_sockets) {
                    const auto& globalTransform = socket.m_nodeIndex >= 0 ? rigNodeTransforms[socket.m_nodeIndex] : ID_MAT;

                    socketPalette[socket.m_index] = socket.calculateGlobalTransform(globalTransform);
                }

                m_jointRegistry.markDirty(registeredRig.m_jointRef);
                m_socketRegistry.markDirty(registeredRig.m_socketRef);
            }
        }
    }

    void AnimateNode::updateAnimatedVolume(
        AnimationState& state,
        model::Node* node)
    {
        auto* type = node->getType();
        const auto& lodMeshes = type->getLodMeshes();
        if (lodMeshes.empty()) return;

        const auto& registeredRigs = node->getRegisteredRigs();

        glm::vec3 minPos{ std::numeric_limits<float>::max() };
        glm::vec3 maxPos{ std::numeric_limits<float>::lowest() };

        // Collect positions from joint nodes
        for (const auto& registeredRig : registeredRigs) {
            const auto* rig = registeredRig.m_rig;

            // Find LodMesh matching this rig to get correct baseTransform
            const glm::mat4* baseTransform = nullptr;
            for (const auto& lodMesh : lodMeshes) {
                if (lodMesh.m_mesh && lodMesh.m_mesh->getRig() == rig) {
                    baseTransform = &lodMesh.m_baseTransform;
                    break;
                }
            }
            if (!baseTransform) continue;

            const auto& rigNodeTransforms = m_rigNodeRegistry.getRange(registeredRig.m_rigRef);
            const auto& jointContainer = rig->getJointContainer();

            for (const auto& joint : jointContainer.m_joints) {
                if (joint.m_nodeIndex < 0) continue;

                // Position in raw mesh space
                //glm::vec4 rawPos = glm::vec4(glm::vec3(rigNodeTransforms[joint.m_nodeIndex][3]), 1.f);
                glm::vec4 rawPos = rigNodeTransforms[joint.m_nodeIndex][3];

                // Transform to match AABB space (with baseScale, rotation, etc.)
                glm::vec3 jointPos = glm::vec3(*baseTransform * rawPos);
                minPos = glm::min(minPos, jointPos);
                maxPos = glm::max(maxPos, jointPos);
            }
        }

        // Skip if no valid joint positions found
        if (minPos.x == std::numeric_limits<float>::max()) return;

        // Calculate center and radius of bounding sphere
        glm::vec3 center = (minPos + maxPos) * 0.5f;
        float radius = glm::length(maxPos - center);

        // Add margin for mesh geometry around joints
        const auto& nodeState = node->getState();
        const auto& originalVolume = nodeState.getLocalVolume();
        float meshMargin = std::max(0.4f, originalVolume.getRadius() * 0.2f);
        radius += meshMargin;

        // Store in AnimationState for SceneUpdater to apply
        {
            std::lock_guard lock(m_volumeLock);
            state.m_animatedVolume = SphereVolume{ center, radius };
            state.m_volumeDirty = true;
        }
    }

    void AnimateNode::updateGroundOffset(
        AnimationState& state,
        model::Node* node)
    {
        auto* type = node->getType();
        const auto& lodMeshes = type->getLodMeshes();
        if (lodMeshes.empty()) return;

        const auto& registeredRigs = node->getRegisteredRigs();

        float minFootY = std::numeric_limits<float>::max();

        for (const auto& registeredRig : registeredRigs) {
            const auto* rig = registeredRig.m_rig;

            // Find LodMesh matching this rig to get correct baseTransform
            const glm::mat4* baseTransform = nullptr;
            for (const auto& lodMesh : lodMeshes) {
                if (lodMesh.m_mesh && lodMesh.m_mesh->getRig() == rig) {
                    baseTransform = &lodMesh.m_baseTransform;
                    break;
                }
            }
            if (!baseTransform) continue;

            const auto& rigNodeTransforms = m_rigNodeRegistry.getRange(registeredRig.m_rigRef);

            // Find ground contact sockets and track lowest Y
            for (const auto& socket : rig->m_sockets) {
                if (socket.m_nodeIndex < 0) continue;
                if (socket.m_role != SocketRole::foot_left &&
                    socket.m_role != SocketRole::foot_right &&
                    socket.m_role != SocketRole::ground_sensor) continue;

                // Socket transform is in raw mesh space, apply baseTransform
                //glm::vec4 rawPos = glm::vec4(glm::vec3(rigNodeTransforms[socket.m_nodeIndex][3]), 1.f);
                glm::vec4 rawPos = rigNodeTransforms[socket.m_nodeIndex][3];
                glm::vec3 footPos = glm::vec3(*baseTransform * rawPos);
                minFootY = std::min(minFootY, footPos.y);
            }
        }

        // Only update if we found foot sockets
        if (minFootY < std::numeric_limits<float>::max()) {
            std::lock_guard lock(m_volumeLock);
            state.m_groundOffsetY = minFootY;
            state.m_groundOffsetDirty = true;
        }
        else {
            state.m_groundOffsetY = 0.f;
            state.m_groundOffsetDirty = true;
        }
    }
}
