#pragma once

#include "GLSyncQueue.h"

namespace kigl {
    //
    // SyncQueue: IMPLEMENTATION
    // - include in xxx.cpp
    //

    template <class T, bool mappedMode>
    GLSyncQueue<T, mappedMode>::GLSyncQueue(
        std::string_view name,
        size_t entryCount,
        size_t rangeCount,
        bool useFence,
        bool useSingleFence)
        : m_rangeCount{ rangeCount },
        m_entryCount{ entryCount },
        m_useFence{ useFence },
        m_useSingleFence{ useSingleFence },
        m_entrySize{ sizeof(T) },
        m_name{ fmt::format("{}_sync_queue", name) },
        m_buffer{ m_name }
    {
    }

    template <class T, bool mappedMode>
    GLSyncQueue<T, mappedMode>::~GLSyncQueue()
    {
        if constexpr (!mappedMode) {
            free(m_data);
        }
    }

    template <class T, bool mappedMode>
    void GLSyncQueue<T, mappedMode>::prepare(int bindAlignment, bool debug)
    {
        m_debug = debug;
        m_bindAlignment = bindAlignment;
        m_rangeLength = m_entryCount * m_entrySize;

        m_paddedRangeLength = m_rangeLength;
        size_t pad = m_rangeLength % m_bindAlignment;
        if (pad > 0) {
            m_paddedRangeLength += (m_bindAlignment - pad);
        }

        if constexpr (mappedMode) {
            // https://cpp-rendering.io/indirect-rendering/
            // - based into slides, use GL_DYNAMIC_STORAGE_BIT for create of mapped buffer
            m_buffer.createEmpty(
                m_rangeCount * m_paddedRangeLength,
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT);

            // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
            // https://stackoverflow.com/questions/44299324/how-to-use-gl-map-unsynchronized-bit-with-gl-map-persistent-bit
            // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
            m_data = m_buffer.map(
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
        }
        else {
            m_buffer.createEmpty(
                m_rangeCount * m_paddedRangeLength,
                GL_DYNAMIC_STORAGE_BIT);
            m_data = (unsigned char*)malloc(m_rangeCount * m_paddedRangeLength);
            if (m_data) {
                memset(m_data, 0, m_rangeCount * m_paddedRangeLength);
            }
        }

        m_ranges.reserve(m_rangeCount);

        for (size_t i = 0; i < m_rangeCount; i++) {
            auto& range = m_ranges.emplace_back();
            // static
            range.m_index = i;
            range.m_baseIndex = i * m_entryCount;
            range.m_maxCount = m_entryCount;
            range.m_entrySize = m_entrySize;
            range.m_baseOffset = i * m_paddedRangeLength;
            range.m_length = m_rangeLength;
            range.m_paddedLength = m_paddedRangeLength;
            // dynamic
            range.m_usedCount = 0;

            if (m_useFence && (m_fences.empty() || !m_useSingleFence)) {
                m_fences.emplace_back(fmt::format("fence_{}_{}", m_name, i));
            }
        }
    }

    template <class T, bool mappedMode>
    bool GLSyncQueue<T, mappedMode>::send(const T& entry)
    {
        waitFence(m_current);

        auto& range = m_ranges[m_current];

        T* ptr = (T*)(m_data + range.nextOffset());

        // NOTE KI memcpy is *likely* faster than assingment operator
        //*ptr = entry;
        memcpy(ptr, &entry, sizeof(T));

        return range.full();
    }

    template <class T, bool mappedMode>
    void GLSyncQueue<T, mappedMode>::set(int idx, const T& entry)
    {
        waitFence(m_current);

        auto& range = m_ranges[m_current];

        //m_data[range.index(idx)] = entry;

        T* ptr = (T*)(m_data + range.offset(idx));
        // NOTE KI memcpy is *likely* faster than assingment operator
        //*ptr = entry;
        memcpy(ptr, &entry, sizeof(T));

        if (idx >= range.m_usedCount) {
            range.m_usedCount = idx + 1;
        }
    }

    template <class T, bool mappedMode>
    void GLSyncQueue<T, mappedMode>::flush()
    {
        const auto& range = m_ranges[m_current];

        if (mappedMode) {
            m_buffer.flushRange(range.m_baseOffset, range.getUsedLength());
        }
        else {
            m_buffer.update(range.m_baseOffset, range.getUsedLength(), m_data + range.m_baseOffset);
        }

        setFence(m_current);
    }

    template <class T, bool mappedMode>
    void GLSyncQueue<T, mappedMode>::processPending(
        std::function<void(GLBufferRange&)> handle,
        bool processCurrent,
        bool clear)
    {
        const size_t untilCount = processCurrent ? m_rangeCount : m_rangeCount - 1;

        for (size_t i = 1; i <= untilCount; i++) {
            size_t rangeIndex = (m_current + i) % m_rangeCount;
            auto& range = m_ranges[rangeIndex];
            if (range.empty()) continue;
            if (rangeIndex == m_current) flush();
            handle(range);
            if (clear) range.clear();
        }
    }

    template <class T, bool mappedMode>
    GLBufferRange& GLSyncQueue<T, mappedMode>::next(bool fence)
    {
        if (fence) {
            setFence(m_current);
        }

        m_current = (m_current + 1) % m_ranges.size();

        return m_ranges[m_current];
    }

    template <class T, bool mappedMode>
    void GLSyncQueue<T, mappedMode>::bind(GLuint ubo, bool used, size_t count)
    {
        auto& range = m_ranges[m_current];
        count = used ? range.m_usedCount : count;
        m_buffer.bindRange(ubo, range.m_baseOffset, range.getLengthFor(count));
    }

    template <class T, bool mappedMode>
    void GLSyncQueue<T, mappedMode>::bindSSBO(GLuint ssbo, bool used, size_t count)
    {
        auto& range = m_ranges[m_current];
        count = used ? range.m_usedCount : count;
        m_buffer.bindSSBORange(ssbo, range.m_baseOffset, range.getLengthFor(count));
    }

    template <class T, bool mappedMode>
    void GLSyncQueue<T, mappedMode>::setFence(size_t index)
    {
        if (!m_useFence) return;

        if (m_useSingleFence) {
            if (index == m_ranges.size() - 1) {
                m_fences[0].setFence(m_debug);
            }
        }
        else {
            m_fences[index].setFence(m_debug);
        }
    }

    template <class T, bool mappedMode>
    void GLSyncQueue<T, mappedMode>::waitFence(size_t index)
    {
        if (!m_useFence) return;

        if (index == 0 || !m_useSingleFence) {
            m_fences[index].waitFence(m_debug);
        }
    }
}
