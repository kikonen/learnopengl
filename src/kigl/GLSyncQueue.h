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
        int entryCount,
        int rangeCount)
        : m_rangeCount(rangeCount),
        m_entryCount(entryCount),
        m_entrySize(sizeof(T)),
        m_rangeSize(entryCount* sizeof(T))
    {
    }

    void prepare() {
        m_buffer.create();

        m_buffer.initEmpty(
            m_rangeCount * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
        m_mapped = (T*)m_buffer.mapRange(
            0,
            m_rangeCount * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

        m_ranges.reserve(m_rangeCount);

        for (int i = 0; i < m_rangeCount; i++) {
            auto& range = m_ranges.emplace_back();
            range.m_baseIndex = i * m_entryCount;
            range.m_maxCount = m_entryCount;
            range.m_count = 0;
            range.m_baseOffset = i * m_rangeSize;
        }
    }

    //
    // Send entry data, wait if needed for queue buffer
    //
    // @return true if current range is full (and ready to flush)
    //
    bool send(T& entry) {
        auto& range = m_ranges[m_index];
        if (m_index == 0) range.waitFence();

        m_mapped[range.next()] = entry;

        return range.isFull();
    }

    //
    // Set entry data, wait if needed for queue buffer
    //
    // NOTE KI with "set" it's upto caller to manage buffer full
    //
    void set(int idx, T& entry) {
        auto& range = m_ranges[m_index];
        if (m_index == 0) range.waitFence();

        m_mapped[range.index(idx)] = entry;
    }

    inline GLBufferRange& current() {
        return m_ranges[m_index];
    }

    //
    // @return true if current range is full
    //
    inline bool isFull() {
        return m_ranges[m_index].isFull();
    }

    //
    // Switch to next range
    //
    inline void next(bool clear) {
        if (clear) {
            m_ranges[m_index].clear();
        }
        m_index = (m_index + 1) % m_ranges.size();
        if (m_index == 0) m_ranges[0].setFence();
    }

public:
    GLBuffer m_buffer;

private:
    const int m_entryCount;
    const int m_entrySize;
    const int m_rangeCount;
    const int m_rangeSize;

    T* m_mapped{ nullptr };

    int m_index = 0;
    std::vector<GLBufferRange> m_ranges;
};

