#include "BoneRegistry.h"

#include "asset/SSBO.h"

#include "animation/RigContainer.h"
#include "animation/BoneTransform.h"

#include "BoneTransformSSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1000;
    constexpr size_t MAX_BLOCK_COUNT = 5100;
}

BoneRegistry& BoneRegistry::get() noexcept
{
    static BoneRegistry s_registry;
    return s_registry;
}

BoneRegistry::BoneRegistry()
{
    // NOTE KI null entry
    m_transforms.emplace_back();
}

BoneRegistry::~BoneRegistry() = default;

void BoneRegistry::prepare()
{
    m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(BoneTransformSSBO), GL_DYNAMIC_STORAGE_BIT);
    m_ssbo.bindSSBO(SSBO_BONE_TRANSFORMS);
}

uint32_t BoneRegistry::registerInstance(animation::RigContainer& rig)
{
    if (!rig.hasBones()) return 0;
    auto index = m_transforms.size();
    m_transforms.resize(m_transforms.size() + rig.m_boneContainer.size());

    snapshotBones();

    return static_cast<uint32_t>(index);
}

inline std::span<animation::BoneTransform> BoneRegistry::modifyRange(uint32_t start, uint32_t count) noexcept
{
    return std::span{ m_transforms }.subspan(start, count);
}

uint32_t BoneRegistry::getActiveBoneCount() const noexcept
{
    return static_cast<uint32_t>(m_snapshot.size());
}

void BoneRegistry::updateWT(const UpdateContext& ctx)
{
}

void BoneRegistry::updateRT(const UpdateContext& ctx)
{
    if (!m_updateReady) return;

    updateBuffer();
}

void BoneRegistry::snapshotBones()
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

void BoneRegistry::updateBuffer()
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
