#pragma once

#include "GLSyncQueue.h"

namespace kigl {
    //
    // SyncQueue: IMPLEMENTATION
    // - include in xxx.cpp
    //

    template <class T>
    GLSyncQueue<T>::GLSyncQueue(
        std::string_view name,
        size_t entryCount,
        size_t rangeCount,
        bool useMapped,
        bool useInvalidate,
        bool useFence,
        bool useFenceDebug)
        : m_rangeCount{ rangeCount },
        m_entryCount{ entryCount },
        m_useMapped{ useMapped },
        m_useInvalidate{ useInvalidate },
        m_useFence{ useFence },
        m_useFenceDebug{ useFenceDebug },
        m_flags{ static_cast<GLuint>(useMapped ? GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT : GL_DYNAMIC_STORAGE_BIT) },
        //m_flags{ static_cast<GLuint>(useMapped ? GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT : GL_DYNAMIC_STORAGE_BIT) },
        // Beyond Porting.pdf
        m_mapFlags{ GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT },
        //m_mapFlags{ GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT },
        m_entrySize{ sizeof(T) },
        m_name{ fmt::format("{}_sync_queue", name) },
        m_buffer{ m_name }
    {
    }

    template <class T>
    GLSyncQueue<T>::~GLSyncQueue()
    {
        if (!m_useMapped) {
            free(m_data);
        }
    }

    template <class T>
    void GLSyncQueue<T>::prepare(
        int bindAlignment,
        bool debug)
    {
        m_debug = debug;
        m_bindAlignment = bindAlignment;
        m_rangeLength = m_entryCount * m_entrySize;

        m_paddedRangeLength = m_rangeLength;
        size_t pad = m_rangeLength % m_bindAlignment;
        if (pad > 0) {
            m_paddedRangeLength += (m_bindAlignment - pad);
        }

        if (m_useMapped) {
            // https://cpp-rendering.io/indirect-rendering/
            // - based into slides, use GL_DYNAMIC_STORAGE_BIT for create of mapped buffer
            m_buffer.createEmpty(
                m_rangeCount * m_paddedRangeLength,
                m_flags);

            // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
            // https://stackoverflow.com/questions/44299324/how-to-use-gl-map-unsynchronized-bit-with-gl-map-persistent-bit
            // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
            m_data = m_buffer.map(m_mapFlags);
        }
        else {
            m_buffer.createEmpty(
                m_rangeCount * m_paddedRangeLength,
                m_flags);
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

            if (m_useFence) {
                m_fences.emplace_back(fmt::format("fence_{}_{}", m_name, i));
            }
        }
    }

    template <class T>
    bool GLSyncQueue<T>::send(const T& entry)
    {
        waitFence();

        auto& range = m_ranges[m_current];

        T* __restrict ptr = (T*)(m_data + range.nextOffset());

        // NOTE KI memcpy is *likely* faster than assingment operator
        //*ptr = entry;
        std::memcpy(ptr, &entry, sizeof(T));

        return range.full();
    }

    template <class T>
    void GLSyncQueue<T>::set(int idx, const T& entry)
    {
        waitFence();

        auto& range = m_ranges[m_current];

        //m_data[range.index(idx)] = entry;

        T* __restrict ptr = (T*)(m_data + range.offset(idx));
        // NOTE KI memcpy is *likely* faster than assingment operator
        //*ptr = entry;
        std::memcpy(ptr, &entry, sizeof(T));

        if (idx >= range.m_usedCount) {
            range.m_usedCount = idx + 1;
        }
    }

    template <class T>
    void GLSyncQueue<T>::flush()
    {
        const auto& range = m_ranges[m_current];

        if (m_useMapped) {
            m_buffer.flushRange(range.m_baseOffset, range.getUsedLength());
        }
        else {
            //if (m_useInvalidate) {
            //    m_buffer.invalidateRange(range.m_baseOffset, range.getUsedLength());
            //}
            m_buffer.update(range.m_baseOffset, range.getUsedLength(), m_data + range.m_baseOffset);
        }
    }

    template <class T>
    void GLSyncQueue<T>::processCurrent(
        std::function<void(GLBufferRange&)> handle)
    {
        auto& range = m_ranges[m_current];
        if (range.empty()) return;
        handle(range);
        clear();
        setFence();
    }

    template <class T>
    GLBufferRange& GLSyncQueue<T>::next()
    {
        m_current = (m_current + 1) % m_ranges.size();

        return m_ranges[m_current];
    }

    template <class T>
    void GLSyncQueue<T>::bindSSBO(GLuint ssbo)
    {
        m_buffer.bindSSBO(ssbo);
    }

    template <class T>
    void GLSyncQueue<T>::bindCurrentUBO(GLuint ubo, bool used, size_t count)
    {
        auto& range = m_ranges[m_current];
        count = used ? range.m_usedCount : count;
        m_buffer.bindUBORange(ubo, range.m_baseOffset, range.getLengthFor(count));
    }

    template <class T>
    void GLSyncQueue<T>::bindCurrentSSBO(GLuint ssbo, bool used, size_t count)
    {
        auto& range = m_ranges[m_current];
        count = used ? range.m_usedCount : count;
        m_buffer.bindSSBORange(ssbo, range.m_baseOffset, range.getLengthFor(count));
    }

    template <class T>
    void GLSyncQueue<T>::setFence()
    {
        if (!m_useFence) return;

        m_fences[m_current].setFence(m_useFenceDebug);
    }

    template <class T>
    bool GLSyncQueue<T>::setFenceIfNotSet()
    {
        if (!m_useFence) return false;

        return m_fences[m_current].setFenceIfNotSet(m_useFenceDebug);
    }

    template <class T>
    void GLSyncQueue<T>::waitFence()
    {
        if (!m_useFence) return;

        m_fences[m_current].waitFence(m_useFenceDebug);
    }
}
