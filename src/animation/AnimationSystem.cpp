#include "AnimationSystem.h"

#include <tuple>
#include <algorithm>
#include <execution>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "asset/Assets.h"
#include "asset/SSBO.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "mesh/MeshType.h"
#include "mesh/ModelMesh.h"
#include "mesh/PrimitiveMesh.h"

#include "pool/NodeHandle.h"

#include "render/DebugContext.h"

#include "animation/RigContainer.h"
#include "animation/BoneInfo.h"
#include "animation/Animator.h"

#include "animation/BoneRegistry.h"
#include "animation/SocketRegistry.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1000;
    constexpr size_t MAX_BLOCK_COUNT = 5100;

    static animation::AnimationSystem s_registry;

    struct ActiveNode {
        animation::AnimationState& m_state;
        Node* m_node;
        mesh::MeshType* m_type;
    };
}

namespace animation
{
    animation::AnimationSystem& AnimationSystem::get() noexcept
    {
        return s_registry;
    }

    AnimationSystem::AnimationSystem()
    {
    }

    AnimationSystem::~AnimationSystem() = default;

    void AnimationSystem::prepare()
    {
        const auto& assets = Assets::get();

        m_enabled = assets.animationEnabled;
        m_onceOnly = assets.animationOnceOnly;
        m_maxCount = assets.animationMaxCount;

        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        boneRegistry.prepare();
        socketRegistry.prepare();
    }

    std::pair<uint32_t, uint32_t> AnimationSystem::registerInstance(const animation::RigContainer& rig)
    {
        std::lock_guard lock(m_pendingLock);

        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        uint32_t boneBaseIndex = boneRegistry.reserveInstance(rig.m_boneContainer.size());
        uint32_t socketBaseIndex = socketRegistry.reserveInstance(rig.m_sockets.size());

        // NOTE KI all bones are initially identity matrix
        // NOTE KI sockets need to be initialiazed to match initial static joint hierarchy
        {
            // NOTE KI need to keep locked while bones are modified
            // => avoid races with registration of other instances
            std::lock_guard lockSockets(socketRegistry.m_lock);

            auto socketPalette = socketRegistry.modifyRange(socketBaseIndex, rig.m_sockets.size());
            for (const auto& socket : rig.m_sockets) {
                const auto& rigJoint = rig.m_joints[socket.m_jointIndex];
                socketPalette[socket.m_index] =
                    socket.m_meshScaleTransform *
                    rigJoint.m_globalTransform *
                    glm::translate(glm::mat4{ 1.f }, socket.m_offset) *
                    glm::toMat4(socket.m_rotation) *
                    socket.m_invMeshScaleTransform;

            }
            socketRegistry.markDirty(socketBaseIndex, rig.m_sockets.size());
        }

        return { boneBaseIndex, socketBaseIndex };
    }

    animation::AnimationState* AnimationSystem::getState(
        pool::NodeHandle handle)
    {
        const auto& it = m_nodeToState.find(handle);
        if (it == m_nodeToState.end()) return nullptr;
        return &m_states[it->second];
    }

    void AnimationSystem::startAnimation(
        pool::NodeHandle handle,
        uint16_t clipIndex,
        float blendTime,
        float speed,
        bool restart,
        bool repeat,
        double startTime)
    {
        auto* state = getState(handle);
        if (!state) return;

        auto& play = state->m_pending;
        play.m_clipIndex = clipIndex;
        play.m_startTime = startTime;
        play.m_blendTime = blendTime;
        play.m_speed = speed;
        play.m_repeat = repeat;
        play.m_active = true;
    }

    void AnimationSystem::stopAnimation(
        pool::NodeHandle handle,
        double stopTime)
    {
        auto* state = getState(handle);
        if (!state) return;

        auto& play = state->m_pending;
        play.m_clipIndex = -1;
        play.m_startTime = stopTime;
        play.m_blendTime = 0.f;
        play.m_speed = 1.f;
        play.m_repeat = false;
        play.m_active = true;
    }

    uint32_t AnimationSystem::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_states.size());
    }

    void AnimationSystem::updateWT(const UpdateContext& ctx)
    {
        prepareNodes();

        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        static std::vector<ActiveNode> s_activeNodes;

        // prepare
        {
            s_activeNodes.clear();
            s_activeNodes.reserve(m_states.size());

            for (auto& state : m_states) {
                auto* node = state.m_handle.toNode();
                if (!node) continue;
                auto* type = node->m_typeHandle.toType();
                s_activeNodes.push_back({ state, node, type });
            }
        }

        // execute
        {
            std::lock_guard lock(m_pendingLock);

            if (m_enabled) {
                // NOTE KI need to keep locked while bones are modified
                // => avoid races with registration of other instances
                std::lock_guard lockBones(boneRegistry.m_lock);
                std::lock_guard lockSockets(socketRegistry.m_lock);

                if (true) {
                    std::for_each(
                        std::execution::par_unseq,
                        s_activeNodes.begin(),
                        s_activeNodes.end(),
                        [this, &ctx](auto& active) {
                            animateNode(ctx, active.m_state, active.m_node, active.m_type);
                        });
                }
                else {
                    for (auto& active : s_activeNodes) {
                        animateNode(ctx, active.m_state, active.m_node, active.m_type);
                    }
                }
            }
        }

        boneRegistry.updateWT();
        socketRegistry.updateWT();
    }

    void AnimationSystem::animateNode(
        const UpdateContext& ctx,
        animation::AnimationState& state,
        Node* node,
        mesh::MeshType* type)
    {
        auto& debugContext = render::DebugContext::modify();
        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        if (debugContext.m_animationPaused) return;

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

            if (!playA.m_active)
                return;
        }

        for (const auto& lodMesh : type->getLodMeshes()) {
            if (!lodMesh.m_flags.useAnimation) continue;

            auto* mesh = lodMesh.getMesh<mesh::Mesh>();
            auto rig = mesh->getRigContainer().get();

            if (!rig) continue;

            uint32_t boneBaseIndex;
            uint32_t socketBaseIndex;
            {
                const auto& nodeState = node->getState();
                boneBaseIndex = nodeState.m_boneBaseIndex;
                socketBaseIndex = nodeState.m_socketBaseIndex;
            }

            auto bonePalette = boneRegistry.modifyRange(boneBaseIndex, rig->m_boneContainer.size());
            auto socketPalette = socketRegistry.modifyRange(socketBaseIndex, rig->m_sockets.size());

            double currentTime = ctx.m_clock.ts;

            float blendFactor = -1.f;

            if (debugContext.m_animationDebugEnabled) {
                playA.m_clipIndex = debugContext.m_animationClipIndexA;
                playB.m_clipIndex = debugContext.m_animationClipIndexB;

                playA.m_speed = debugContext.m_animationSpeedA;
                playB.m_speed = debugContext.m_animationSpeedB;

                blendFactor = debugContext.m_animationBlendFactor;

                if (debugContext.m_animationManualTime) {
                    currentTime = debugContext.m_animationCurrentTime;
                    playA.m_startTime = debugContext.m_animationStartTimeA;
                    playB.m_startTime = debugContext.m_animationStartTimeB;
                }
                else {
                    if (playA.m_startTime < 1000.f) {
                        playA.m_startTime = currentTime;
                    }
                    if (playB.m_startTime < 1000.f) {
                        playB.m_startTime = currentTime + 3.f;
                    }
                }

                if (!debugContext.m_animationBlend) {
                    playB.m_clipIndex = -1;
                    playB.m_active = false;
                }
            }

            if (playB.m_active) {
                if (blendFactor < 0) {
                    if (playB.m_blendTime > 0) {
                        auto diff = currentTime - playB.m_startTime;
                        blendFactor = diff / playB.m_blendTime;
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

            animation::Animator animator;
            bool changed = false;
             if (!playB.m_active) {
                 changed = animator.animate(
                     *rig,
                     mesh->m_rigTransform,
                     mesh->m_inverseRigTransform,
                     lodMesh.m_animationRigTransform,
                     bonePalette,
                     socketPalette,
                     playA.m_clipIndex,
                     playA.m_startTime,
                     playA.m_speed,
                     currentTime,
                     debugContext.m_animationForceFirstFrame);
            }
            else {
                 changed = animator.animateBlended(
                     *rig,
                     mesh->m_rigTransform,
                     mesh->m_inverseRigTransform,
                     lodMesh.m_animationRigTransform,
                     bonePalette,
                     socketPalette,
                     playA.m_clipIndex,
                     playA.m_startTime,
                     playA.m_speed,
                     playB.m_clipIndex,
                     playB.m_startTime,
                     playB.m_speed,
                     blendFactor,
                     currentTime,
                     debugContext.m_animationForceFirstFrame);
            }

            if (m_onceOnly) {
                playA.m_active = false;
                playB.m_active = false;
            }

            if (changed) {
                boneRegistry.markDirty(boneBaseIndex, bonePalette.size());
                socketRegistry.markDirty(socketBaseIndex, socketPalette.size());
            }

            // NOTE KI need to animated only once
            // => multiple rigs per node are *NOT* currently supported
            break;
        }
    }

    void AnimationSystem::updateRT(const UpdateContext& ctx)
    {
        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        boneRegistry.updateRT();
        socketRegistry.updateRT();
    }

    void AnimationSystem::prepareNodes()
    {
        std::lock_guard lock(m_pendingLock);
        if (m_pendingNodes.empty()) return;

        for (auto& handle : m_pendingNodes) {
            uint16_t index = static_cast<uint16_t>(m_states.size());
            auto& state = m_states.emplace_back(handle);
            state.m_index = index;
            m_nodeToState.insert({ handle, state.m_index });
        }
        m_pendingNodes.clear();
    }

    void AnimationSystem::handleNodeAdded(Node* node)
    {
        if (!m_enabled) return;

        auto* type = node->m_typeHandle.toType();

        if (!type->m_flags.anyAnimation) return;

        std::lock_guard lock(m_pendingLock);
        m_pendingNodes.push_back(node->toHandle());
    }
}
