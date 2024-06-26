#include "AnimationSystem.h"

#include <tuple>
#include <algorithm>
#include <execution>

#include "asset/Assets.h"
#include "asset/SSBO.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "mesh/MeshType.h"
#include "mesh/ModelMesh.h"

#include "pool/NodeHandle.h"

#include "animation/RigContainer.h"
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
        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        uint32_t boneIndex = boneRegistry.reserveInstance(rig.m_boneContainer.size());
        uint32_t socketIndex = socketRegistry.reserveInstance(rig.m_sockets.size());

        return { boneIndex, socketIndex };
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

    bool AnimationSystem::animateNode(
        const UpdateContext& ctx,
        Node* node,
        mesh::MeshType* type)
    {
        bool result = false;

        auto& boneRegistry = BoneRegistry::get();
        auto& socketRegistry = SocketRegistry::get();

        for (const auto& lodMesh : type->getLodMeshes()) {
            if (!lodMesh.m_flags.useAnimation) continue;

            const auto* mesh = lodMesh.getMesh<mesh::ModelMesh>();
            auto& state = node->modifyState();

            if (state.m_animationStartTime <= -42.f) {
                continue;
            }

            auto& rig = *mesh->m_rig;
            auto palette = boneRegistry.modifyRange(state.m_boneBaseIndex, rig.m_boneContainer.size());
            auto sockets = socketRegistry.modifyRange(state.m_socketBaseIndex, rig.m_sockets.size());

            if (state.m_animationStartTime < 0) {
                state.m_animationStartTime = ctx.m_clock.ts - (rand() % 60);
            }
            if (rig.m_animations.size() > 1) {
                state.m_animationIndex = 1;
            }

            animation::Animator animator;
            auto changed = animator.animate(
                ctx,
                rig,
                mesh->m_baseTransform,
                mesh->m_inverseBaseTransform,
                lodMesh.m_animationBaseTransform,
                palette,
                sockets,
                state.m_animationIndex,
                state.m_animationStartTime,
                m_firstFrameOnly ? state.m_animationStartTime : ctx.m_clock.ts);

            if (m_onceOnly) {
                state.m_animationStartTime = -42;
            }

            if (changed) {
                boneRegistry.markDirty(state.m_boneBaseIndex, rig.m_boneContainer.size());
                socketRegistry.markDirty(state.m_socketBaseIndex, rig.m_sockets.size());
            }
            result |= changed;
        }
        return result;
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
