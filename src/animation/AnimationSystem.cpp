#include "AnimationSystem.h"

#include <tuple>
#include <algorithm>
#include <execution>
#include <unordered_map>

#include <fmt/format.h>

#include "ki/RenderClock.h"

#include "util/thread.h"
#include "util/glm_format.h"
#include "util/BufferReference.h"

#include "asset/Assets.h"

#include "shader/SSBO.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "mesh/ModelMesh.h"
#include "mesh/PrimitiveMesh.h"
#include "mesh/LodMesh.h"
#include "mesh/LodMeshInstance.h"
#include "mesh/RegisteredRig.h"

#include "pool/NodeHandle.h"

#include "debug/DebugContext.h"

#include "animation/Rig.h"
#include "animation/RigNode.h"
#include "animation/JointContainer.h"
#include "animation/Joint.h"

#include "animation/RigNodeRegistry.h"
#include "animation/SocketRegistry.h"
#include "animation/JointRegistry.h"

#include "animation/JointBuffer.h"
#include "animation/SocketBuffer.h"
#include "animation/AnimateNode.h"

namespace {
    //constexpr size_t BLOCK_SIZE = 1000;
    //constexpr size_t MAX_BLOCK_COUNT = 5100;

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
        //m_socketBuffer->prepare();

        m_animateNode = std::make_unique<AnimateNode>(
            *m_rigNodeRegistry,
            *m_jointRegistry,
            *m_socketRegistry,
            m_volumeLock,
            m_onceOnly);

        clear();
    }

    void AnimationSystem::beginFrame()
    {
        m_jointBuffer->beginFrame();
        m_socketBuffer->beginFrame();
    }

    void AnimationSystem::endFrame()
    {
        m_jointBuffer->endFrame();
        m_socketBuffer->endFrame();
    }

    util::BufferReference AnimationSystem::registerRig(
        const animation::Rig& rig)
    {
        std::lock_guard lock(m_pendingLock);

        auto& rigNodeRegistry = *m_rigNodeRegistry;

        util::BufferReference rigRef = rigNodeRegistry.allocate(rig.m_nodes.size());

        if (rigRef.size > 0) {
            // Initialize joints
            if (rigRef.offset) {
                auto rigNodeTransforms = rigNodeRegistry.modifyRange(rigRef);

                for (const auto& rigNode : rig.m_nodes) {
                    const auto& parentTransform = rigNode.m_index > 0 ? rigNodeTransforms[rigNode.m_parentIndex] : ID_MAT;
                    rigNodeTransforms[rigNode.m_index] = parentTransform * rigNode.m_transform;
                }
            }
        }

        return rigRef;
    }

    util::BufferReference AnimationSystem::registerSockets(
        util::BufferReference rigRef,
        const animation::Rig& rig)
    {
        std::lock_guard lock(m_pendingLock);

        auto& rigNodeRegistry = *m_rigNodeRegistry;
        auto& socketRegistry = *m_socketRegistry;

        util::BufferReference socketRef = socketRegistry.allocate(rig.m_sockets.size());

        // NOTE KI all joints are initially identity matrix
        // NOTE KI sockets need to be initialiazed to match initial static joint hierarchy
        if (socketRef.size > 0) {
            const auto rigNodeTransforms = rigNodeRegistry.getRange(rigRef);

            // NOTE KI need to keep locked while joints are modified
            // => avoid races with registration of other instances
            std::lock_guard lockSockets(socketRegistry.m_lock);

            auto socketPalette = socketRegistry.modifyRange(socketRef);
            for (const auto& socket : rig.m_sockets) {
                // NOTE KI see Animator::animate()
                const auto& globalTransform = socket.m_nodeIndex >= 0 ? rigNodeTransforms[socket.m_nodeIndex] : ID_MAT;
                socketPalette[socket.m_index] = socket.calculateGlobalTransform(globalTransform);
            }
            socketRegistry.markDirty(socketRef);
        }

        return socketRef;
    }

    util::BufferReference AnimationSystem::registerJoints(
        util::BufferReference rigRef,
        const animation::JointContainer& jointContainer)
    {
        std::lock_guard lock(m_pendingLock);

        auto& rigNodeRegistry = *m_rigNodeRegistry;
        auto& jointRegistry = *m_jointRegistry;

        util::BufferReference jointRef = jointRegistry.allocate(jointContainer.size());

        // Initialize joints
        if (jointRef.size > 0) {
            std::lock_guard lockjoints(jointRegistry.m_lock);

            const auto rigNodeTransforms = rigNodeRegistry.getRange(rigRef);
            auto jointPalette = jointRegistry.modifyRange(jointRef);

            glm::mat4 inverseMeshRigTransform{ 1.f };

            for (const auto& joint : jointContainer.m_joints) {
                const auto& globalTransform = joint.m_nodeIndex >= 0 ? rigNodeTransforms[joint.m_nodeIndex] : ID_MAT;

                jointPalette[joint.m_jointIndex] = inverseMeshRigTransform * globalTransform * joint.m_offsetMatrix;
            }

            jointRegistry.markDirty(jointRef);
        }

        return jointRef;
    }

    util::BufferReference AnimationSystem::unregisterRig(
        util::BufferReference rigRef)
    {
        std::lock_guard lock(m_pendingLock);

        auto& rigNodeRegistry = *m_rigNodeRegistry;

        rigNodeRegistry.release(rigRef);

        return { 0, 0 };
    }

    util::BufferReference AnimationSystem::unregisterSockets(
        util::BufferReference socketRef)
    {
        std::lock_guard lock(m_pendingLock);

        auto& socketRegistry = *m_socketRegistry;

        socketRegistry.release(socketRef);

        return { 0, 0 };
    }

    util::BufferReference AnimationSystem::unregisterJoints(
        util::BufferReference jointRef)
    {
        std::lock_guard lock(m_pendingLock);

        auto& jointRegistry = *m_jointRegistry;

        jointRegistry.release(jointRef);

        return { 0, 0 };
    }

    const glm::mat4& AnimationSystem::getSocketTransform(
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
                            m_animateNode->animate(ctx, active.m_state, active.m_node);
                        });
                }
                else {
                    for (auto& active : s_activeNodes) {
                        m_animateNode->animate(ctx, active.m_state, active.m_node);
                    }
                }
            }
        }

        rigNodeRegistry.updateWT();
        jointRegistry.updateWT();
        socketRegistry.updateWT();
    }

    void AnimationSystem::updateRT(const UpdateContext& ctx)
    {
        ASSERT_RT();

        auto& jointBuffer = *m_jointBuffer;
        //auto& socketBuffer = *m_socketBuffer;

        jointBuffer.updateRT();
        //socketBuffer.updateRT();
    }

    void AnimationSystem::applyAnimatedVolumes()
    {
        auto& nodeRegistry = NodeRegistry::get();

        std::lock_guard lock(m_volumeLock);

        for (auto& animState : m_states) {
            if (!animState.m_volumeDirty) continue;

            //auto* node = animState.m_handle.toNode();
            //if (!node) continue;

            auto& nodeState = nodeRegistry.modifyState(animState.m_handle.m_handleIndex);
            nodeState.setLocalVolume(animState.m_animatedVolume);
            animState.m_volumeDirty = false;
        }
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
