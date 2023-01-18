#pragma once

#include <vector>

#include "GLBuffer.h"
#include "GLBufferRange.h"

//
// SyncQueue, which is split into multiple ranges, which can be fence synced
//
template <class T, bool mappedMode>
class GLSyncQueue {
public:
    GLSyncQueue(
        std::string name,
        int entryCount,
        int rangeCount)
        : m_rangeCount(rangeCount),
        m_entryCount(entryCount),
        m_entrySize(sizeof(T)),
        m_buffer{ "syncQueue_" + name}
    {
    }

    ~GLSyncQueue() {
        if constexpr (!mappedMode) {
            free(m_data);
        }
    }

    void prepare(int bindAlignment) {
        m_bindAlignment = bindAlignment;
        m_rangeLength = m_entryCount * m_entrySize;

        m_paddedRangeLength = m_rangeLength;
        int pad = m_rangeLength % m_bindAlignment;
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
            range.m_baseIndex = i * m_entryCount;
            range.m_maxCount = m_entryCount;
            range.m_entrySize = m_entrySize;
            range.m_baseOffset = i * m_paddedRangeLength;
            range.m_length = m_rangeLength;
            range.m_paddedLength = m_paddedRangeLength;
            // dynamic
            range.m_count = 0;
        }
    }

    //
    // Send entry data, wait if needed for queue buffer
    //
    // @return true if current range is full (and ready to flush)
    //
    bool send(const T& entry) {
        auto& range = m_ranges[m_current];
        if (m_current == 0) range.waitFence();

        //m_data[range.next()] = entry;
        T* ptr = (T*)(m_data + range.nextOffset());
        *ptr = entry;

        return range.isFull();
    }

    //
    // Set entry data, wait if needed for queue buffer
    //
    // NOTE KI with "set" it's upto caller to manage buffer full
    //
    void set(int idx, T& entry) {
        auto& range = m_ranges[m_current];
        if (m_current == 0) range.waitFence();

        //m_data[range.index(idx)] = entry;
        T* ptr = (T*)(m_data + range.offset(idx));
        *ptr = entry;
    }

    inline GLBufferRange& current() {
        return m_ranges[m_current];
    }

    //
    // @return true if current range is full
    //
    inline bool isFull() {
        return m_ranges[m_current].isFull();
    }

    inline bool isEmpty() {
        return m_ranges[m_current].isEmpty();
    }

    inline void clear() {
        m_ranges[m_current].clear();
    }

    inline void flush() {
        const auto& range = m_ranges[m_current];
        if (mappedMode) {
            m_buffer.flushRange(range.m_baseOffset, range.getUsedSize());
        }
        else {
            m_buffer.update(range.m_baseOffset, range.getUsedSize(), m_data + range.m_baseOffset);
        }
    }

    //
    // Switch to next range
    //
    inline void next(bool clear) {
        if (clear) {
            m_ranges[m_current].clear();
        }
        m_current = (m_current + 1) % m_ranges.size();
        if (m_current == 0) m_ranges[0].setFence();
    }

    inline void bind(GLuint ubo) {
        auto& range = m_ranges[m_current];
        m_buffer.bindRange(ubo, range.m_baseOffset, range.m_length);
    }

public:
    GLBuffer m_buffer;

private:
    const int m_entryCount;
    const int m_entrySize;
    const int m_rangeCount;

    int m_bindAlignment = 0;
    int m_rangeLength = 0;
    int m_paddedRangeLength = 0;

    unsigned char* m_data{ nullptr };

    int m_current = 0;
    std::vector<GLBufferRange> m_ranges;
};

