#include "JointBuffer.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "shader/SSBO.h"

#include "kigl/GLBuffer.h"

#include "JointRegistry.h"
#include "JointTransformSSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1024;
    constexpr size_t MAX_BLOCK_COUNT = 5100;

    constexpr size_t MAX_JOINT_COUNT = BLOCK_SIZE * MAX_BLOCK_COUNT;
}

namespace animation
{
    JointBuffer::JointBuffer(JointRegistry* jointRegistry)
        : m_jointRegistry{ jointRegistry }
    {
    }

    JointBuffer::~JointBuffer() = default;

    void JointBuffer::clear()
    {
        ASSERT_RT();
    }

    void JointBuffer::prepare()
    {
        ASSERT_RT();

        // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
        m_ssbo.createEmpty(BLOCK_SIZE * sizeof(JointTransformSSBO), kigl::getBufferStorageFlags());
        m_ssbo.map(kigl::getBufferMapFlags());

        m_ssbo.bindSSBO(SSBO_JOINT_TRANSFORMS);
    }

    void JointBuffer::beginFrame()
    {
        m_fence.waitFence();
    }

    void JointBuffer::endFrame()
    {
        m_fence.setFence();
    }

    void JointBuffer::updateRT()
    {
        if (!m_jointRegistry->m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < 2) {
            return;
        }
        m_frameSkipCount = 0;

        upload();
    }

    void JointBuffer::upload()
    {
        std::lock_guard lock(m_jointRegistry->m_lock);

        if (m_jointRegistry->m_dirtySnapshot.empty()) return;

        auto totalCount = m_jointRegistry->m_snapshot.size();
        resizeBuffer(totalCount);

        for (const auto& range : m_jointRegistry->m_dirtySnapshot) {
            uploadSpan(m_jointRegistry->m_snapshot, range);
        }

        m_jointRegistry->m_dirtySnapshot.clear();
        m_jointRegistry->m_updateReady = false;
    }

    void JointBuffer::uploadSpan(
        const std::vector<JointTransformSSBO>& snapshot,
        const util::BufferReference& range)
    {
        if (range.size == 0) return;
        if (range.offset >= snapshot.size()) return;

        const size_t count = std::min(
            static_cast<size_t>(range.size),
            snapshot.size() - range.offset);

        auto* __restrict mappedData = m_ssbo.mapped<JointTransformSSBO>(0);

        std::copy_n(
            snapshot.data() + range.offset,
            count,
            mappedData + range.offset);

        // NOTE KI flush for explicit mode (no-op if using coherent mapping)
        m_ssbo.flushRange(range.offset * sizeof(JointTransformSSBO), count * sizeof(JointTransformSSBO));
    }

    void JointBuffer::resizeBuffer(size_t totalCount)
    {
        if (m_entryCount >= totalCount) return;

        size_t blocks = (totalCount / BLOCK_SIZE) + 2;
        size_t entryCount = blocks * BLOCK_SIZE;

        if (entryCount > MAX_JOINT_COUNT) {
            KI_CRITICAL(fmt::format("ERROR: MAX_JOINT_COUNT reached, size={}", entryCount));
            entryCount = std::min(entryCount, MAX_JOINT_COUNT);
        }

        // NOTE KI *reallocate* SSBO if needed
        m_ssbo.resizeBuffer(entryCount * sizeof(JointTransformSSBO), true);

        m_ssbo.map(kigl::getBufferMapFlags());

        m_ssbo.bindSSBO(SSBO_JOINT_TRANSFORMS);

        m_entryCount = entryCount;
    }
}
