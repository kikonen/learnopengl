#include "BoneBuffer.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "shader/SSBO.h"

#include "BoneRegistry.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1000;
    constexpr size_t MAX_BLOCK_COUNT = 5100;
}

namespace animation
{
    BoneBuffer::BoneBuffer(BoneRegistry* boneRegistry)
        : m_boneRegistry{ boneRegistry }
    {
    }

    BoneBuffer::~BoneBuffer() = default;

    void BoneBuffer::clear()
    {
        ASSERT_RT();
    }

    void BoneBuffer::shutdown()
    {
        ASSERT_RT();

        clear();
    }

    void BoneBuffer::prepare()
    {
        ASSERT_RT();

        const auto& assets = Assets::get();

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useDebugFence = assets.glUseDebugFence;

        m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(glm::mat4), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_BONE_TRANSFORMS);
    }

    void BoneBuffer::updateRT()
    {
        if (!m_boneRegistry->m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < 2) {
            return;
        }
        m_frameSkipCount = 0;

        updateBuffer();
    }

    void BoneBuffer::updateBuffer()
    {
        std::lock_guard lock(m_boneRegistry->m_lock);

        if (m_boneRegistry->m_dirtySnapshot.empty()) return;

        for (const auto& range : m_boneRegistry->m_dirtySnapshot) {
            if (updateSpan(
                m_boneRegistry->m_snapshot,
                range.first,
                range.second)) break;
        }

        m_boneRegistry->m_dirtySnapshot.clear();
        m_boneRegistry->m_updateReady = false;
    }

    bool BoneBuffer::updateSpan(
        const std::vector<glm::mat4>& snapshot,
        size_t updateIndex,
        size_t updateCount)
    {
        constexpr size_t sz = sizeof(glm::mat4);
        const size_t totalCount = snapshot.size();

        if (totalCount == 0) return true;

        if (m_ssbo.m_size < totalCount * sz) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t bufferSize = blocks * BLOCK_SIZE * sz;
            if (m_ssbo.resizeBuffer(bufferSize)) {
                m_ssbo.bindSSBO(SSBO_BONE_TRANSFORMS);
            }

            updateIndex = 0;
            updateCount = totalCount;
        }

        //if (m_useInvalidate) {
        //    m_ssbo.invalidateRange(updateIndex * sz, updateCount * sz);
        //}

        m_ssbo.update(
            updateIndex * sz,
            updateCount * sz,
            &snapshot[updateIndex]);

        return updateCount == totalCount;
    }
}
