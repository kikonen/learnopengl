#include "AnimationSystem.h"

#include <tuple>
#include <algorithm>
#include <execution>

#include <fmt/format.h>

#include "ki/RenderClock.h"

#include "util/thread.h"
#include "util/glm_format.h"

#include "asset/Assets.h"

#include "shader/SSBO.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "mesh/ModelMesh.h"
#include "mesh/PrimitiveMesh.h"

#include "pool/NodeHandle.h"

#include "debug/DebugContext.h"

#include "animation/RigContainer.h"
#include "animation/Joint.h"
#include "animation/Animator.h"

#include "animation/RigNodeRegistry.h"
#include "animation/SocketRegistry.h"
#include "animation/JointRegistry.h"

#include "animation/JointBuffer.h"
#include "animation/SocketBuffer.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1000;
    constexpr size_t MAX_BLOCK_COUNT = 5100;

    static const glm::mat4 ID_MAT{ 1.f };

    struct ActiveNode {
        animation::AnimationState& m_state;
        model::Node* m_node;
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
        : m_rigNodeRegistry{ std::make_unique<RigNodeRegistry>() },
        m_jointRegistry{ std::make_unique<JointRegistry>() },
        m_socketRegistry{ std::make_unique<SocketRegistry>() },
        m_jointBuffer{ std::make_unique<JointBuffer>(m_jointRegistry.get()) },
        m_socketBuffer{ std::make_unique<SocketBuffer>(m_socketRegistry.get()) }
    {
    }

    AnimationSystem::~AnimationSystem() = default;

    void AnimationSystem::clear()
    {
        ASSERT_RT();

        m_states.clear();
        m_nodeToState.clear();

        m_pendingNodes.clear();

        m_rigNodeRegistry->clear();
        m_jointRegistry->clear();
        m_socketRegistry->clear();

        m_jointBuffer->clear();
        m_socketBuffer->clear();
    }

    void AnimationSystem::prepare()
    {
        ASSERT_RT();

        const auto& assets = Assets::get();

        m_enabled = assets.animationEnabled;
        m_onceOnly = assets.animationOnceOnly;
        m_maxCount = assets.animationMaxCount;

        m_rigNodeRegistry->prepare();
        m_socketRegistry->prepare();
        m_jointRegistry->prepare();

        m_jointBuffer->prepare();
        m_socketBuffer->prepare();

        clear();
    }

    std::tuple<uint32_t, uint32_t, uint32_t> AnimationSystem::registerInstance(
        const animation::RigContainer& rig)
    {
        std::lock_guard lock(m_pendingLock);

        auto& rigNodeRegistry = *m_rigNodeRegistry;
        auto& jointRegistry = *m_jointRegistry;
        auto& socketRegistry = *m_socketRegistry;

        uint32_t rigNodeBaseIndex = rigNodeRegistry.addInstance(rig.m_nodes.size());
        uint32_t jointBaseIndex = jointRegistry.addInstance(rig.m_jointContainer.size());
        uint32_t socketBaseIndex = socketRegistry.addInstance(rig.m_sockets.size());

        {
            std::lock_guard lockjoints(jointRegistry.m_lock);

            // Initialize joints
            if (rigNodeBaseIndex) {
                auto rigNodeTransforms = rigNodeRegistry.modifyRange(rigNodeBaseIndex, rig.m_nodes.size());

                for (const auto& rigNode : rig.m_nodes) {
                    const auto& parentTransform = rigNode.m_index > 0 ? rigNodeTransforms[rigNode.m_parentIndex] : ID_MAT;
                    rigNodeTransforms[rigNode.m_index] = parentTransform * rigNode.m_transform;
                }

                jointRegistry.markDirty(jointBaseIndex, rig.m_jointContainer.size());
            }

            // Initialize joints
            if (jointBaseIndex) {
                const auto rigNodeTransforms = rigNodeRegistry.modifyRange(rigNodeBaseIndex, rig.m_nodes.size());
                auto jointPalette = jointRegistry.modifyRange(jointBaseIndex, rig.m_jointContainer.size());

                glm::mat4 inverseMeshRigTransform{ 1.f };

                for (const auto& rigNode : rig.m_nodes) {
                    const auto* joint = rig.m_jointContainer.getJoint(rigNode.m_jointIndex);
                    if (joint) {
                        const auto& globalTransform = rigNodeTransforms[rigNode.m_index];
                        jointPalette[joint->m_index] = inverseMeshRigTransform * globalTransform * joint->m_offsetMatrix;
                    }
                }
                jointRegistry.markDirty(jointBaseIndex, rig.m_jointContainer.size());
            }
        }

        // NOTE KI all joints are initially identity matrix
        // NOTE KI sockets need to be initialiazed to match initial static joint hierarchy
        if (socketBaseIndex)
        {
            const auto rigNodeTransforms = rigNodeRegistry.modifyRange(rigNodeBaseIndex, rig.m_nodes.size());

            // NOTE KI need to keep locked while joints are modified
            // => avoid races with registration of other instances
            std::lock_guard lockSockets(socketRegistry.m_lock);

            auto socketPalette = socketRegistry.modifyRange(socketBaseIndex, rig.m_sockets.size());
            for (const auto& socket : rig.m_sockets) {
                // NOTE KI see Animator::animate()
                const auto& rigNode = rig.m_nodes[socket.m_nodeIndex];
                const auto& globalTransform = rigNodeTransforms[rigNode.m_index];
                socketPalette[socket.m_index] = socket.calculateGlobalTransform(globalTransform);
            }
            socketRegistry.markDirty(socketBaseIndex, rig.m_sockets.size());
        }

        return { rigNodeBaseIndex, jointBaseIndex, socketBaseIndex };
    }

    void AnimationSystem::unregisterInstance(
        const animation::RigContainer& rig,
        uint32_t rigNodeBaseIndex,
        uint32_t jointBaseIndex,
        uint32_t socketBaseIndex)
    {
        std::lock_guard lock(m_pendingLock);

        auto& rigNodeRegistry = *m_rigNodeRegistry;
        auto& socketRegistry = *m_socketRegistry;
        auto& jointRegistry = *m_jointRegistry;

        rigNodeRegistry.removeInstance(rigNodeBaseIndex, rig.m_nodes.size());
        jointRegistry.removeInstance(jointBaseIndex, rig.m_jointContainer.size());
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

        auto& rigNodeRegistry = *m_rigNodeRegistry;
        auto& socketRegistry = *m_socketRegistry;
        auto& jointRegistry = *m_jointRegistry;

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
                // NOTE KI need to keep locked while joints are modified
                // => avoid races with registration of other instances
                std::lock_guard lockjoints(jointRegistry.m_lock);
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

        rigNodeRegistry.updateWT();
        jointRegistry.updateWT();
        socketRegistry.updateWT();
    }

    void AnimationSystem::animateNode(
        const UpdateContext& ctx,
        animation::AnimationState& state,
        model::Node* node)
    {
        const auto& dbg = debug::DebugContext::modify();
        const auto& anim = dbg.m_animation;

        auto& rigNodeRegistry = *m_rigNodeRegistry;
        auto& jointRegistry = *m_jointRegistry;
        auto& socketRegistry = *m_socketRegistry;

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

        for (const auto& lodMesh : node->getLodMeshes()) {
            if (!lodMesh.m_flags.useAnimation) continue;

            auto* mesh = lodMesh.getMesh<mesh::VaoMesh>();
            if (!mesh) continue;
            if (mesh->m_rigNodeIndex < 0) continue;

            // TDOO KI handle case when same rig is used for multiple
            // meshes (i.e. meshes possibly split due to material, etc.)
            auto rig = mesh->getRigContainer().get();
            if (!rig) continue;

            uint32_t rigNodeBaseIndex;
            uint32_t jointBaseIndex;
            uint32_t socketBaseIndex;
            {
                const auto& nodeState = node->getState();
                rigNodeBaseIndex = nodeState.m_rigNodeBaseIndex;
                jointBaseIndex = nodeState.m_jointBaseIndex;
                socketBaseIndex = nodeState.m_socketBaseIndex;
            }

            auto nodeTransforms = rigNodeRegistry.modifyRange(rigNodeBaseIndex, rig->m_nodes.size());
            auto jointPalette = jointRegistry.modifyRange(jointBaseIndex, rig->m_jointContainer.size());
            auto socketPalette = socketRegistry.modifyRange(socketBaseIndex, rig->m_sockets.size());

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

            animation::Animator animator;
            bool changed = false;
             if (!playB.m_active) {
                 changed = animator.animate(
                     *rig,
                     glm::mat4{ 1.f },
                     nodeTransforms,
                     jointPalette,
                     socketPalette,
                     playA.m_clipIndex,
                     playA.m_startTime,
                     playA.m_speed,
                     currentTime,
                     anim.m_forceFirstFrame);
            }
            else {
                 changed = animator.animateBlended(
                     *rig,
                     glm::mat4{ 1.f },
                     nodeTransforms,
                     jointPalette,
                     socketPalette,
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
                rigNodeRegistry.markDirty(rigNodeBaseIndex, nodeTransforms.size());
                jointRegistry.markDirty(jointBaseIndex, jointPalette.size());
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

        auto& jointBuffer = *m_jointBuffer;
        auto& socketBuffer = *m_socketBuffer;

        jointBuffer.updateRT();
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

    void AnimationSystem::handleNodeAdded(model::Node* node)
    {
        if (!m_enabled) return;

        if (!node->m_typeFlags.anyAnimation) return;

        std::lock_guard lock(m_pendingLock);
        m_pendingNodes.push_back(node->toHandle());
    }

    void AnimationSystem::handleNodeRemoved(model::Node* node)
    {
        if (!m_enabled) return;
        if (!node->m_typeFlags.anyAnimation) return;

        auto nodeHandle = node->toHandle();

        std::lock_guard lock(m_pendingLock);

        nodeHandle.removeFrom(m_pendingNodes);
        m_nodeToState.erase(nodeHandle);
    }
}
