#include "SocketBuffer.h"

#include "util/thread.h"

#include "asset/Assets.h"

#include "kigl/GLSyncQueue_impl.h"

#include "shader/SSBO.h"

#include "SocketRegistry.h"
#include "SocketTransformSSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1024;
    constexpr size_t MAX_BLOCK_COUNT = 5100;

    constexpr size_t RANGE_COUNT = 3;
}

namespace animation
{
    SocketBuffer::SocketBuffer(SocketRegistry* socketRegistry)
        : m_socketRegistry{ socketRegistry }
    {
    }

    SocketBuffer::~SocketBuffer() = default;

    void SocketBuffer::clear()
    {
        ASSERT_RT();
    }

    void SocketBuffer::shutdown()
    {
        ASSERT_RT();

        clear();
    }

    void SocketBuffer::prepare()
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
        m_useFenceDebug = true;

        m_frameSkipCount = 1;
    }

    void SocketBuffer::updateRT()
    {
        if (!m_socketRegistry->m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < 2) {
            return;
        }
        m_frameSkipCount = 0;

        updateBuffer();
    }

    void SocketBuffer::updateBuffer()
    {
        std::lock_guard lock(m_socketRegistry->m_lock);

        if (m_socketRegistry->m_dirtySnapshot.empty()) return;

        //for (const auto& range : m_socketRegistry->m_dirtySnapshot) {
        //    if (updateSpan(
        //        m_socketRegistry->m_snapshot,
        //        range.first,
        //        range.second)) break;
        //}

        auto totalCount = m_socketRegistry->m_snapshot.size();
        createBuffer(totalCount);
        updateSpan(
            m_socketRegistry->m_snapshot,
            0,
            totalCount);

        m_socketRegistry->m_dirtySnapshot.clear();
        m_socketRegistry->m_updateReady = false;
    }

    void SocketBuffer::createBuffer(size_t totalCount)
    {
        if (!m_queue || m_queue->getEntryCount() < totalCount) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t entryCount = blocks * BLOCK_SIZE;

            // NOTE KI OpenGL Insights - Chapter 28
            m_queue = std::make_unique<kigl::GLSyncQueue<SocketTransformSSBO>>(
                "socket_ssbo",
                entryCount,
                RANGE_COUNT,
                m_useMapped,
                m_useInvalidate,
                m_useFence,
                m_useFenceDebug);

            m_queue->prepare(1, false);
        }
    }

    bool SocketBuffer::updateSpan(
        const std::vector<SocketTransformSSBO>& snapshot,
        size_t updateIndex,
        size_t updateCount)
    {
        constexpr size_t sz = sizeof(SocketTransformSSBO);
        const size_t totalCount = snapshot.size();

        if (totalCount == 0) return true;

        auto& current = m_queue->current();
        auto* mappedData = m_queue->currentMapped();

        m_queue->waitFence();
        std::copy(
            std::begin(snapshot),
            std::end(snapshot),
            mappedData);
        m_queue->setFence();

        m_queue->bindCurrentSSBO(SSBO_SOCKET_TRANSFORMS, false, totalCount);

        m_queue->next();

        return updateCount == totalCount;
    }
}
