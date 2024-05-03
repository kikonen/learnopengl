#include "AnimationSystem.h"

#include "asset/SSBO.h"

#include "animation/RigContainer.h"
#include "animation/BoneTransform.h"

#include "animation/BoneTransformSSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1000;
    constexpr size_t MAX_BLOCK_COUNT = 5100;
}

namespace animation
{
    animation::AnimationSystem& AnimationSystem::get() noexcept
    {
        static AnimationSystem s_registry;
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
        m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(BoneTransformSSBO), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_BONE_TRANSFORMS);
    }

    uint32_t AnimationSystem::registerInstance(animation::RigContainer& rig)
    {
        if (!rig.hasBones()) return 0;
        auto index = m_transforms.size();
        m_transforms.resize(m_transforms.size() + rig.m_boneContainer.size());

        snapshotBones();

        return static_cast<uint32_t>(index);
    }

    inline std::span<animation::BoneTransform> AnimationSystem::modifyRange(uint32_t start, uint32_t count) noexcept
    {
        return std::span{ m_transforms }.subspan(start, count);
    }

    uint32_t AnimationSystem::getActiveBoneCount() const noexcept
    {
        return static_cast<uint32_t>(m_snapshot.size());
    }

    void AnimationSystem::updateWT(const UpdateContext& ctx)
    {
    }

    void AnimationSystem::updateRT(const UpdateContext& ctx)
    {
        if (!m_updateReady) return;

        updateBuffer();
    }

    void AnimationSystem::snapshotBones()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_transforms.empty()) {
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

        m_ssbo.update(
            0,
            totalCount * sz,
            m_snapshot.data());

        m_updateReady = false;
    }
}
