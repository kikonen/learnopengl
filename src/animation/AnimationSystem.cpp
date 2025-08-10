#include "AnimationSystem.h"

#include <tuple>
#include <algorithm>
#include <execution>

#include <fmt/format.h>

#include "util/thread.h"
#include "util/glm_format.h"

#include "asset/Assets.h"

#include "shader/SSBO.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "mesh/ModelMesh.h"
#include "mesh/PrimitiveMesh.h"

#include "pool/NodeHandle.h"

#include "render/DebugContext.h"

#include "animation/RigContainer.h"
#include "animation/BoneInfo.h"
#include "animation/Animator.h"

#include "animation/BoneRegistry.h"
#include "animation/SocketRegistry.h"

#include "animation/BoneBuffer.h"
#include "animation/SocketBuffer.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1000;
    constexpr size_t MAX_BLOCK_COUNT = 5100;

    struct ActiveNode {
        animation::AnimationState& m_state;
        Node* m_node;
    };

    static animation::AnimationSystem* s_system{ nullptr };
}

namespace animation
{
    void AnimationSystem::init() noexcept
    {
        assert(!s_system);
        s_system = new AnimationSystem();
    }

    void AnimationSystem::release() noexcept
    {
        auto* s = s_system;
        s_system = nullptr;
        delete s;
    }

    AnimationSystem& AnimationSystem::get() noexcept
    {
        assert(s_system);
        return *s_system;
    }
}

namespace animation
{
    AnimationSystem::AnimationSystem()
        : m_boneRegistry{ std::make_unique<BoneRegistry>() },
        m_socketRegistry{ std::make_unique<SocketRegistry>() },
        m_boneBuffer{ std::make_unique<BoneBuffer>(m_boneRegistry.get()) },
        m_socketBuffer{ std::make_unique<SocketBuffer>(m_socketRegistry.get()) }
    {
    }

    AnimationSystem::~AnimationSystem() = default;

    void AnimationSystem::clearWT()
    {
        ASSERT_WT();

        m_states.clear();
        m_nodeToState.clear();

        m_pendingNodes.clear();

        m_boneRegistry->clear();
        m_socketRegistry->clear();
    }

    void AnimationSystem::shutdownWT()
    {
        ASSERT_WT();

        clearWT();
    }

    void AnimationSystem::prepareWT()
    {
        ASSERT_WT();

        const auto& assets = Assets::get();

        m_enabled = assets.animationEnabled;
        m_onceOnly = assets.animationOnceOnly;
        m_maxCount = assets.animationMaxCount;

        m_boneRegistry->prepare();
        m_socketRegistry->prepare();

        clearWT();
    }

    void AnimationSystem::clearRT()
    {
        ASSERT_RT();

        m_boneBuffer->clear();
        m_socketBuffer->clear();
    }

    void AnimationSystem::shutdownRT()
    {
        ASSERT_RT();

        clearRT();
    }

    void AnimationSystem::prepareRT()
    {
        ASSERT_RT();

        m_boneBuffer->prepare();
        m_socketBuffer->prepare();

        clearRT();
    }

    std::pair<uint32_t, uint32_t> AnimationSystem::registerInstance(
        const animation::RigContainer& rig)
    {
        std::lock_guard lock(m_pendingLock);

        auto& boneRegistry = *m_boneRegistry;
        auto& socketRegistry = *m_socketRegistry;

        uint32_t boneBaseIndex = boneRegistry.addInstance(rig.m_boneContainer.size());
        uint32_t socketBaseIndex = socketRegistry.addInstance(rig.m_sockets.size());

        // Initialize bones
        if (false) {
            std::lock_guard lockBones(boneRegistry.m_lock);

            auto bonePalette = boneRegistry.modifyRange(boneBaseIndex, rig.m_boneContainer.size());

            std::vector<glm::mat4> parentTransforms;
            parentTransforms.resize(rig.m_joints.size() + 1);
            parentTransforms[0] = glm::mat4{ 1.f };

            for (const auto& rigJoint : rig.m_joints) {

                const auto& jointTransform = rigJoint.m_transform;
                parentTransforms[rigJoint.m_index + 1] = parentTransforms[rigJoint.m_parentIndex + 1] * jointTransform;

                const auto* bone = rig.m_boneContainer.getInfo(rigJoint.m_boneIndex);
                if (bone) {
                    const auto& globalTransform = parentTransforms[rigJoint.m_index + 1];
                    bonePalette[bone->m_index] = globalTransform * bone->m_offsetMatrix;
                }
            }
            boneRegistry.markDirty(boneBaseIndex, rig.m_boneContainer.size());
        }

        // NOTE KI all bones are initially identity matrix
        // NOTE KI sockets need to be initialiazed to match initial static joint hierarchy
        {
            // NOTE KI need to keep locked while bones are modified
            // => avoid races with registration of other instances
            std::lock_guard lockSockets(socketRegistry.m_lock);

            auto socketPalette = socketRegistry.modifyRange(socketBaseIndex, rig.m_sockets.size());
            for (const auto& socket : rig.m_sockets) {
                // NOTE KI see Animator::animate()
                const auto& rigJoint = rig.m_joints[socket.m_jointIndex];
                socketPalette[socket.m_index] = socket.calculateGlobalTransform(rigJoint.m_globalTransform);
            }
            socketRegistry.markDirty(socketBaseIndex, rig.m_sockets.size());
        }

        return { boneBaseIndex, socketBaseIndex };
    }

    void AnimationSystem::unregisterInstance(
        const animation::RigContainer& rig,
        uint32_t boneBaseIndex,
        uint32_t socketBaseIndex)
    {
        std::lock_guard lock(m_pendingLock);

        auto& boneRegistry = *m_boneRegistry;
        auto& socketRegistry = *m_socketRegistry;

        boneRegistry.removeInstance(boneBaseIndex, rig.m_boneContainer.size());
        socketRegistry.removeInstance(socketBaseIndex, rig.m_sockets.size());
    }

    glm::mat4 AnimationSystem::getSocketTransform(
        uint32_t index) const noexcept
    {
        return m_socketRegistry->getTransform(index);
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
        waitForPrepared();

        auto* state = getState(handle);
        if (!state) {
            KI_WARN_OUT("ANIM: STATE_MISSING");
            return;
        }

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

        auto& boneRegistry = *m_boneRegistry;
        auto& socketRegistry = *m_socketRegistry;

        static std::vector<ActiveNode> s_activeNodes;

        // prepare
        {
            s_activeNodes.clear();
            s_activeNodes.reserve(m_states.size());

            for (auto& state : m_states) {
                auto* node = state.m_handle.toNode();
                if (!node) continue;
                s_activeNodes.push_back({ state, node });
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
                            animateNode(ctx, active.m_state, active.m_node);
                        });
                }
                else {
                    for (auto& active : s_activeNodes) {
                        animateNode(ctx, active.m_state, active.m_node);
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
        Node* node)
    {
        auto& dbg = render::DebugContext::modify();

        auto& boneRegistry = *m_boneRegistry;
        auto& socketRegistry = *m_socketRegistry;

        if (dbg.m_animationPaused) return;

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

            if (!playA.m_active && !dbg.m_animationDebugEnabled)
                return;
        }

        for (const auto& lodMesh : node->getLodMeshes()) {
            if (!lodMesh.m_flags.useAnimation) continue;

            auto* mesh = lodMesh.getMesh<mesh::VaoMesh>();
            if (!mesh) continue;

            // TDOO KI handle case when same rig is used for multiple
            // meshes (i.e. meshes possibly split due to material, etc.)
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

            if (dbg.m_animationDebugEnabled) {
                playA.m_clipIndex = dbg.m_animationClipIndexA;
                playB.m_clipIndex = dbg.m_animationClipIndexB;

                playA.m_speed = dbg.m_animationSpeedA;
                playB.m_speed = dbg.m_animationSpeedB;

                blendFactor = dbg.m_animationBlendFactor;

                if (dbg.m_animationManualTime) {
                    currentTime = dbg.m_animationCurrentTime;
                    playA.m_startTime = dbg.m_animationStartTimeA;
                    playB.m_startTime = dbg.m_animationStartTimeB;
                }
                else {
                    if (playA.m_startTime < 1000.f) {
                        playA.m_startTime = currentTime;
                    }
                    if (playB.m_startTime < 1000.f) {
                        playB.m_startTime = currentTime + 3.f;
                    }
                }

                if (!dbg.m_animationBlend) {
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

            animation::Animator animator;
            bool changed = false;
             if (!playB.m_active) {
                 changed = animator.animate(
                     *rig,
                     mesh->m_rigTransform,
                     mesh->m_inverseRigTransform,
                     bonePalette,
                     socketPalette,
                     playA.m_clipIndex,
                     playA.m_startTime,
                     playA.m_speed,
                     currentTime,
                     dbg.m_animationForceFirstFrame);
            }
            else {
                 changed = animator.animateBlended(
                     *rig,
                     mesh->m_rigTransform,
                     mesh->m_inverseRigTransform,
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
                     dbg.m_animationForceFirstFrame);
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
        ASSERT_RT();

        auto& boneBuffer = *m_boneBuffer;
        auto& socketBuffer = *m_socketBuffer;

        boneBuffer.updateRT();
        socketBuffer.updateRT();
    }

    void AnimationSystem::waitForPrepared()
    {
        std::unique_lock<std::mutex> lock(m_pendingLock);

        bool done = m_pendingNodes.empty();

        while (!done) {
            m_pendingWait.wait(lock);
            done = m_pendingNodes.empty();
        }
    }

    void AnimationSystem::prepareNodes()
    {
        std::unique_lock lock(m_pendingLock);

        if (!m_pendingNodes.empty())
        {
            for (auto& handle : m_pendingNodes) {
                uint16_t index = static_cast<uint16_t>(m_states.size());
                auto& state = m_states.emplace_back(handle);
                state.m_index = index;
                m_nodeToState.insert({ handle, state.m_index });
            }
            m_pendingNodes.clear();
        }

        m_pendingWait.notify_all();
    }

    void AnimationSystem::handleNodeAdded(Node* node)
    {
        if (!m_enabled) return;

        if (!node->m_typeFlags.anyAnimation) return;

        std::lock_guard lock(m_pendingLock);
        m_pendingNodes.push_back(node->toHandle());
    }

    void AnimationSystem::handleNodeRemoved(Node* node)
    {
        if (!m_enabled) return;
        if (!node->m_typeFlags.anyAnimation) return;

        auto nodeHandle = node->toHandle();

        std::lock_guard lock(m_pendingLock);

        nodeHandle.removeFrom(m_pendingNodes);
        m_nodeToState.erase(nodeHandle);
    }
}
