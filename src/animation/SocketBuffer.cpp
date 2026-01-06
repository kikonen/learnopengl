#include "SocketBuffer.h"

#include "util/thread.h"

#include "asset/Assets.h"

#include "shader/SSBO.h"

#include "SocketRegistry.h"
#include "SocketTransformSSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1024;
    constexpr size_t MAX_BLOCK_COUNT = 5100;

    constexpr size_t MAX_SOCKET_COUNT = BLOCK_SIZE * MAX_BLOCK_COUNT;
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

    void SocketBuffer::prepare()
    {
        ASSERT_RT();

        m_frameSkipCount = 1;

        // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
        GLuint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        m_ssbo.createEmpty(BLOCK_SIZE * sizeof(SocketTransformSSBO), flags);
        m_ssbo.map(flags);

        //m_ssbo.bindSSBO(SSBO_SOCKET_TRANSFORMS);
    }

    void SocketBuffer::beginFrame()
    {
        m_fence.waitFence();
    }

    void SocketBuffer::endFrame()
    {
        m_fence.setFence();
    }

    void SocketBuffer::updateRT()
    {
        if (!m_socketRegistry->m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < 2) {
            return;
        }
        m_frameSkipCount = 0;

        upload();
    }

    void SocketBuffer::upload()
    {
        std::lock_guard lock(m_socketRegistry->m_lock);

        if (m_socketRegistry->m_dirtySnapshot.empty()) return;

        auto totalCount = m_socketRegistry->m_snapshot.size();
        resizeBuffer(totalCount);

        for (const auto& range : m_socketRegistry->m_dirtySnapshot) {
            uploadSpan(m_socketRegistry->m_snapshot, range);
        }

        m_socketRegistry->m_dirtySnapshot.clear();
        m_socketRegistry->m_updateReady = false;
    }

    void SocketBuffer::uploadSpan(
        const std::vector<SocketTransformSSBO>& snapshot,
        const util::BufferReference& range)
    {
        if (range.size == 0) return;
        if (range.offset >= snapshot.size()) return;

        const size_t count = std::min(
            static_cast<size_t>(range.size),
            snapshot.size() - range.offset);

        auto* __restrict mappedData = m_ssbo.mapped<SocketTransformSSBO>(0);

        std::copy_n(
            snapshot.data() + range.offset,
            count,
            mappedData + range.offset);
    }

    void SocketBuffer::resizeBuffer(size_t totalCount)
    {
        if (m_entryCount >= totalCount) return;

        size_t blocks = (totalCount / BLOCK_SIZE) + 2;
        size_t entryCount = blocks * BLOCK_SIZE;

        if (entryCount > MAX_SOCKET_COUNT) {
            KI_CRITICAL(fmt::format("ERROR: MAX_SOCKET_COUNT reached, size={}", entryCount));
            entryCount = std::min(entryCount, MAX_SOCKET_COUNT);
        }

        // NOTE KI *reallocate* SSBO if needed
        m_ssbo.resizeBuffer(entryCount * sizeof(SocketTransformSSBO), true);

        GLuint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        m_ssbo.map(flags);

        //m_ssbo.bindSSBO(SSBO_SOCKET_TRANSFORMS);

        m_entryCount = entryCount;
    }
}
