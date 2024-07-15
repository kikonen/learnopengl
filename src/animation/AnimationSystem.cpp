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
        m_firstFrameOnly = assets.animationFirstFrameOnly;
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

    uint32_t AnimationSystem::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_animationNodes.size());
    }

    void AnimationSystem::updateWT(const UpdateContext& ctx)
    {
        prepareNodes();

        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        static std::vector<std::pair<Node*, mesh::MeshType*>> s_activeNodes;

        // prepare
        {
            s_activeNodes.clear();
            s_activeNodes.reserve(m_animationNodes.size());

            for (auto& handle : m_animationNodes) {
                auto* node = handle.toNode();
                if (!node) continue;
                auto* type = node->m_typeHandle.toType();

                s_activeNodes.push_back({ node, type });
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
                        [this, &ctx](auto& pair) {
                            animateNode(ctx, pair.first, pair.second);
                        });
                }
                else {
                    for (auto& pair : s_activeNodes) {
                        animateNode(ctx, pair.first, pair.second);
                    }
                }
            }
        }

        boneRegistry.updateWT();
        socketRegistry.updateWT();
    }

    void AnimationSystem::animateNode(
        const UpdateContext& ctx,
        Node* node,
        mesh::MeshType* type)
    {
        auto& debugContext = render::DebugContext::modify();
        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        for (const auto& lodMesh : type->getLodMeshes()) {
            if (!lodMesh.m_flags.useAnimation) continue;

            auto& state = node->modifyState();
            if (state.m_animationStartTime <= -42.f) {
                // NOTE KI "once"
                continue;
            }

            const mesh::Mesh* mesh{ nullptr };
            std::shared_ptr<animation::RigContainer> rigPtr;

            if (!rigPtr)
            {
                const auto* m = lodMesh.getMesh<mesh::ModelMesh>();
                if (m) {
                    rigPtr = m->m_rig;
                    mesh = m;
                }
            }
            if (!rigPtr)
            {
                const auto* m = lodMesh.getMesh<mesh::PrimitiveMesh>();
                if (m) {
                    rigPtr = m->m_rig;
                    mesh = m;
                }
            }

            if (!rigPtr) continue;
            auto& rig = *rigPtr;

            auto bonePalette = boneRegistry.modifyRange(state.m_boneBaseIndex, rig.m_boneContainer.size());
            auto socketPalette = socketRegistry.modifyRange(state.m_socketBaseIndex, rig.m_sockets.size());

            bool paused = false;

            if (debugContext.m_animationDebugEnabled) {
                paused = debugContext.m_animationPaused;

                if (state.m_animationStartTime < 0) {
                    state.m_animationStartTime = ctx.m_clock.ts - (rand() % 60);
                }
                state.m_animationClipIndex = debugContext.m_animationClipIndex;
            }

            double animationStartTime = state.m_animationStartTime;
            double animationCurrentTime = ctx.m_clock.ts;

            if (m_firstFrameOnly) {
                animationCurrentTime = state.m_animationStartTime;
            }
            if (paused) {
                animationCurrentTime = state.m_animationLastTime;
            }

            if (debugContext.m_animationDebugEnabled) {
                if (debugContext.m_animationTime >= 0) {
                    animationStartTime = 0;
                    animationCurrentTime = debugContext.m_animationTime;

                    auto clipIndex = state.m_animationClipIndex;
                    const auto& clipContainer = rig.m_clipContainer;
                    if (clipIndex >= 0 || clipIndex < clipContainer.m_clips.size()) {
                        const auto& clip = clipContainer.m_clips[clipIndex];
                        if (!clip.m_single && animationCurrentTime > clip.m_durationSecs) {
                            animationCurrentTime = clip.m_durationSecs;
                            debugContext.m_animationTime = clip.m_durationSecs;
                        }
                    }
                }
            }

            animation::Animator animator;
            auto changed = animator.animate(
                ctx,
                rig,
                mesh->m_rigTransform,
                mesh->m_inverseRigTransform,
                lodMesh.m_animationRigTransform,
                bonePalette,
                socketPalette,
                state.m_animationClipIndex,
                animationStartTime,
                animationCurrentTime);

            state.m_animationLastTime = animationCurrentTime;
            if (m_onceOnly) {
                state.m_animationStartTime = -42;
            }

            if (changed) {
                boneRegistry.markDirty(state.m_boneBaseIndex, rig.m_boneContainer.size());
                socketRegistry.markDirty(state.m_socketBaseIndex, rig.m_sockets.size());
            }
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
            m_animationNodes.push_back(handle);
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
