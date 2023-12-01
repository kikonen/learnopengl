#pragma once

#include "kigl/kigl.h"

struct GLBufferRange {
    size_t m_maxCount = 0;
    size_t m_usedCount = 0;

    size_t m_index = 0;
    size_t m_entrySize = 0;
    size_t m_baseIndex = 0;
    size_t m_baseOffset = 0;
    size_t m_length = 0;
    size_t m_paddedLength = 0;

    inline size_t nextOffset() {
        return m_baseOffset + (m_entrySize * m_usedCount++);
    }

    inline size_t offset(size_t idx) const {
        return m_baseOffset + (m_entrySize * idx);
    }

    inline bool full() const {
        return m_usedCount == m_maxCount;
    }

    inline bool empty() const {
        return m_usedCount == 0;
    }

    inline size_t getUsedLength() const {
        return m_entrySize * m_usedCount;
    }

    inline size_t getLengthFor(size_t count) const {
        return m_entrySize * count;
    }

    void clear() {
        m_usedCount = 0;
    }
};
