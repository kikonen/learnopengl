#pragma once

#include <string>
#include <vector>
#include <functional>

#include "GLBuffer.h"
#include "GLBufferRange.h"

//
// SyncQueue, which is split into multiple ranges, which can be fence synced
//
template <class T, bool mappedMode>
class GLSyncQueue {
public:
    GLSyncQueue(
        std::string_view name,
        size_t entryCount,
        size_t rangeCount,
        bool useFence)
        : m_rangeCount{ rangeCount },
        m_entryCount{ entryCount },
        m_useFence{ useFence },
        m_entrySize{ sizeof(T) },
        m_buffer{ std::string{ "syncQueue_" } + std::string{ name } }
    {
    }

    ~GLSyncQueue() {
        if constexpr (!mappedMode) {
            free(m_data);
        }
    }

    void prepare(int bindAlignment, bool debug) {
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
            m_data = m_buffer.map(
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
        }
        else {
            m_buffer.createEmpty(
                m_rangeCount * m_paddedRangeLength,
                GL_DYNAMIC_STORAGE_BIT);
            m_data = (unsigned char*)malloc(m_rangeCount * m_paddedRangeLength);
            memset(m_data, 0, m_rangeCount * m_paddedRangeLength);
        }

        m_ranges.reserve(m_rangeCount);

        for (size_t i = 0; i < m_rangeCount; i++) {
            auto& range = m_ranges.emplace_back();
            // static
            range.m_debug = debug;
            range.m_index = i;
            range.m_baseIndex = i * m_entryCount;
            range.m_maxCount = m_entryCount;
            range.m_entrySize = m_entrySize;
            range.m_baseOffset = i * m_paddedRangeLength;
            range.m_length = m_rangeLength;
            range.m_paddedLength = m_paddedRangeLength;
            // dynamic
            range.m_usedCount = 0;
        }
    }

    //
    // Send entry data, wait if needed for queue buffer
    //
    // @return true if current range is full (and ready to flush)
    //
    bool send(const T& entry) {
        auto& range = m_ranges[m_current];

        if (m_useFence)
            range.waitFence();

        T* ptr = (T*)(m_data + range.nextOffset());

        // NOTE KI memcpy is *likely* faster than assingment operator
        //*ptr = entry;
        memcpy(ptr, &entry, sizeof(T));

        return range.full();
    }

    //
    // Set entry data, wait if needed for queue buffer
    //
    // NOTE KI with "set" it's upto caller to manage buffer full
    //
    void set(int idx, T& entry) {
        auto& range = m_ranges[m_current];

        if (m_useFence)
            range.waitFence();

        //m_data[range.index(idx)] = entry;

        T* ptr = (T*)(m_data + range.offset(idx));
        // NOTE KI memcpy is *likely* faster than assingment operator
        //*ptr = entry;
        memcpy(ptr, &entry, sizeof(T));

        if (idx >= range.m_usedCount) {
            range.m_usedCount = idx + 1;
        }
    }

    inline GLBufferRange& current() {
        return m_ranges[m_current];
    }

    //
    // @return true if current range is full
    //
    inline bool full() {
        return m_ranges[m_current].full();
    }

    inline bool empty() {
        return m_ranges[m_current].empty();
    }

    inline void clear() {
        m_ranges[m_current].clear();
    }

    inline void flush() {
        const auto& range = m_ranges[m_current];
        if (mappedMode) {
            m_buffer.flushRange(range.m_baseOffset, range.getUsedLength());
        }
        else {
            m_buffer.update(range.m_baseOffset, range.getUsedLength(), m_data + range.m_baseOffset);
        }
    }

    //
    // Process starting from oldest upto current
    //
    inline void processPending(
        std::function<void(GLBufferRange&)> handle,
        bool processCurrent,
        bool clear)
    {
        const int untilCount = processCurrent ? m_rangeCount : m_rangeCount - 1;

        for (int i = 1; i <= untilCount; i++) {
            int rangeIndex = (m_current + i) % m_rangeCount;
            auto& range = m_ranges[rangeIndex];
            if (range.empty()) continue;
            if (rangeIndex == m_current) flush();
            handle(range);
            if (clear) range.clear();
        }
    }

    //
    // Switch to next range
    // @return next buffer
    //
    inline GLBufferRange& next(bool clear, bool fence) {
        if (clear) {
            m_ranges[m_current].clear();
        }

        if (m_useFence)
            if (fence)
                m_ranges[m_current].setFence();

        m_current = (m_current + 1) % m_ranges.size();

        return m_ranges[m_current];
    }

    inline void bind(GLuint ubo, bool used, int count) {
        auto& range = m_ranges[m_current];
        count = used ? range.m_usedCount : count;
        m_buffer.bindRange(ubo, range.m_baseOffset, range.getLengthFor(count));
    }

    inline void bindSSBO(GLuint ssbo, bool used, int count) {
        auto& range = m_ranges[m_current];
        count = used ? range.m_usedCount : count;
        m_buffer.bindSSBORange(ssbo, range.m_baseOffset, range.getLengthFor(count));
    }

public:
    GLBuffer m_buffer;

private:
    const size_t m_entryCount;
    const size_t m_rangeCount;
    const bool m_useFence;

    const size_t m_entrySize;

    size_t m_bindAlignment = 0;
    size_t m_rangeLength = 0;
    size_t m_paddedRangeLength = 0;

    unsigned char* m_data{ nullptr };

    size_t m_current = 0;
    std::vector<GLBufferRange> m_ranges;
};

