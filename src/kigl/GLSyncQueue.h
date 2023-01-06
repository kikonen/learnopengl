#pragma once

#include <vector>

#include "GLBuffer.h"
#include "GLBufferRange.h"

//
// SyncQueue, which is split into multiple ranges, which can be fence synced
//
template <class T>
class GLSyncQueue {
public:
    GLSyncQueue(
        std::string name,
        int entryCount,
        int rangeCount)
        : m_rangeCount(rangeCount),
        m_entryCount(entryCount),
        m_entrySize(sizeof(T)),
        m_rangeSize(entryCount* sizeof(T)),
        m_buffer{ "syncQueue_" + name}
    {
    }

    void prepare() {
        m_buffer.createEmpty(
            m_rangeCount * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
        // https://stackoverflow.com/questions/44299324/how-to-use-gl-map-unsynchronized-bit-with-gl-map-persistent-bit
        m_data = (T*)m_buffer.mapRange(
            0,
            m_rangeCount * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        m_ranges.reserve(m_rangeCount);

        for (int i = 0; i < m_rangeCount; i++) {
            auto& range = m_ranges.emplace_back();
            range.m_index = i * m_entryCount;
            range.m_maxCount = m_entryCount;
            range.m_count = 0;
            range.m_offset = i * m_rangeSize;
            range.m_length = m_rangeSize;
        }
    }

    //
    // Send entry data, wait if needed for queue buffer
    //
    // @return true if current range is full (and ready to flush)
    //
    bool send(T& entry) {
        auto& range = m_ranges[m_current];
        if (m_current == 0) range.waitFence();

        m_data[range.next()] = entry;

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

        m_data[range.index(idx)] = entry;
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
        m_buffer.bindRange(ubo, range.m_offset, range.m_length);
    }

public:
    GLBuffer m_buffer;

private:
    const int m_entryCount;
    const int m_entrySize;
    const int m_rangeCount;
    const int m_rangeSize;

    T* m_data{ nullptr };

    int m_current = 0;
    std::vector<GLBufferRange> m_ranges;
};

