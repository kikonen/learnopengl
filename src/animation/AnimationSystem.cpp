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
#include "animation/BoneTransform.h"
#include "animation/Animator.h"

#include "animation/BoneTransformSSBO.h"

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
        // NOTE KI null entry
        m_transforms.emplace_back();
    }

    AnimationSystem::~AnimationSystem() = default;

    void AnimationSystem::prepare()
    {
        const auto& assets = Assets::get();

        m_enabled = assets.animationEnabled;
        m_firstFrameOnly = assets.animationFirstFrameOnly;
        m_maxCount = assets.animationMaxCount;

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useDebugFence = assets.glUseDebugFence;

        m_frameSkipCount = 1;

        m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(BoneTransformSSBO), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_BONE_TRANSFORMS);
    }

    uint32_t AnimationSystem::registerInstance(animation::RigContainer& rig)
    {
        if (!rig.hasBones()) return 0;

        size_t index;
        {
            std::lock_guard lock(m_pendingLock);

            index = m_transforms.size();
            m_transforms.resize(m_transforms.size() + rig.m_boneContainer.size());
            for (int i = 0; i < rig.m_boneContainer.size(); i++) {
                m_transforms[index + i] = glm::mat4{ 1.f };
            }

            m_needSnapshot = true;
        }

        return static_cast<uint32_t>(index);
    }

    inline std::span<animation::BoneTransform> AnimationSystem::modifyRange(
        uint32_t start,
        uint32_t count) noexcept
    {
        return std::span{ m_transforms }.subspan(start, count);
    }

    uint32_t AnimationSystem::getActiveBoneCount() const noexcept
    {
        return static_cast<uint32_t>(m_snapshot.size());
    }

    void AnimationSystem::updateWT(const UpdateContext& ctx)
    {
        prepareNodes();

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
                    m_needSnapshot |= true;
                }
                else {
                    bool needSnapshot = false;
                    for (auto& pair : s_activeNodes) {
                        needSnapshot |= animateNode(ctx, pair.first, pair.second);
                    }
                    m_needSnapshot |= needSnapshot;
                }
            }

            if (m_needSnapshot) {
                snapshotBones();
                m_needSnapshot = false;
            }
        }
    }

    bool AnimationSystem::animateNode(
        const UpdateContext& ctx,
        Node* node,
        mesh::MeshType* type)
    {
        const auto* lodMesh = type->getLodMesh(0);
        const auto* mesh = lodMesh->getMesh<mesh::ModelMesh>();
        auto& transform = node->modifyTransform();

        auto& rig = *mesh->m_rig;
        auto palette = modifyRange(transform.m_boneIndex, rig.m_boneContainer.size());

        if (transform.m_animationStartTime < 0) {
            transform.m_animationStartTime = ctx.m_clock.ts - (rand() % 60);
        }
        if (rig.m_animations.size() > 1) {
            transform.m_animationIndex = 1;
        }

        animation::Animator animator;
        return animator.animate(
            ctx,
            rig,
            mesh->m_baseTransform,
            mesh->m_inverseBaseTransform,
            mesh->m_animationBaseTransform,
            palette,
            transform.m_animationIndex,
            transform.m_animationStartTime,
            m_firstFrameOnly ? transform.m_animationStartTime : ctx.m_clock.ts);
    }

    void AnimationSystem::updateRT(const UpdateContext& ctx)
    {
        if (!m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < 2) {
            return;
        }
        m_frameSkipCount = 0;

        updateBuffer();
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
        if (!type->m_flags.useAnimation) return;

        std::lock_guard lock(m_pendingLock);
        m_pendingNodes.push_back(node->toHandle());
    }

    void AnimationSystem::snapshotBones()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_transforms.empty() || !m_needSnapshot) {
            return;
        }

        const size_t totalCount = m_transforms.size();

        if (m_snapshot.size() != totalCount) {
            m_snapshot.resize(totalCount);
        }

        for (size_t i = 0; i < totalCount; i++) {
            m_snapshot[i].u_transform = m_transforms[i].m_transform;
        }

        m_updateReady = true;
    }

    void AnimationSystem::updateBuffer()
    {
        std::lock_guard lock(m_snapshotLock);

        constexpr size_t sz = sizeof(BoneTransformSSBO);
        const size_t totalCount = m_snapshot.size();

        if (m_ssbo.m_size < totalCount * sz) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t bufferSize = blocks * BLOCK_SIZE * sz;
            if (m_ssbo.resizeBuffer(bufferSize)) {
                m_ssbo.bindSSBO(SSBO_BONE_TRANSFORMS);
            }
        }

        //m_ssbo.invalidateRange(
        //    0,
        //    totalCount * sz);

        if (m_useInvalidate) {
            m_ssbo.invalidateRange(0, totalCount * sz);
        }

        m_ssbo.update(
            0,
            totalCount * sz,
            m_snapshot.data());

        m_updateReady = false;
    }
}
