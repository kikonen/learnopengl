#include "BoneBuffer.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "kigl/GLSyncQueue_impl.h"

#include "shader/SSBO.h"

#include "BoneRegistry.h"
#include "BoneTransformSSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1024;
    constexpr size_t MAX_BLOCK_COUNT = 5100;

    constexpr size_t RANGE_COUNT = 3;
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
        m_useFenceDebug = assets.glUseFenceDebug;

        m_useMapped = true;
        m_useInvalidate = false;
        m_useFence = true;
        //m_useFenceDebug = true;
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

        //for (const auto& range : m_boneRegistry->m_dirtySnapshot) {
        //    if (updateSpan(
        //        m_boneRegistry->m_snapshot,
        //        range.first,
        //        range.second)) break;
        //}

        auto totalCount = m_boneRegistry->m_snapshot.size();
        createBuffer(totalCount);
        updateSpan(
            m_boneRegistry->m_snapshot,
            0,
            totalCount);

        m_boneRegistry->m_dirtySnapshot.clear();
        m_boneRegistry->m_updateReady = false;
    }

    void BoneBuffer::createBuffer(size_t totalCount)
    {
        if (!m_queue || m_queue->getEntryCount() < totalCount) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t entryCount = blocks * BLOCK_SIZE;

            // NOTE KI OpenGL Insights - Chapter 28
            m_queue = std::make_unique<kigl::GLSyncQueue<BoneTransformSSBO>>(
                "bone_ssbo",
                entryCount,
                RANGE_COUNT,
                m_useMapped,
                m_useInvalidate,
                m_useFence,
                m_useFenceDebug);

            m_queue->prepare(1, false);
        }
    }

    bool BoneBuffer::updateSpan(
        const std::vector<BoneTransformSSBO>& snapshot,
        size_t updateIndex,
        size_t updateCount)
    {
        constexpr size_t sz = sizeof(BoneTransformSSBO);
        const size_t totalCount = snapshot.size();

        if (totalCount == 0) return true;

        auto& current = m_queue->current();
        auto* __restrict mappedData = m_queue->currentMapped();

        m_queue->waitFence();
        std::copy(
            std::begin(snapshot),
            std::end(snapshot),
            mappedData);
        m_queue->setFence();

        m_queue->bindCurrentSSBO(SSBO_BONE_TRANSFORMS, false, totalCount);

        m_queue->next();

        return updateCount == totalCount;
    }
}
