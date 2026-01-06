#include "GLBuffer.h"

#include <algorithm>

namespace kigl
{
    void GLBuffer::swap(GLBuffer& o) noexcept
    {
        std::swap(m_name, o.m_name);
        std::swap(m_id, o.m_id);
        std::swap(m_size, o.m_size);
        std::swap(m_flags, o.m_flags);
        std::swap(m_usedSize, o.m_usedSize);
        std::swap(m_binding, o.m_binding);
        std::swap(m_mappedData, o.m_mappedData);
        std::swap(m_created, o.m_created);
        std::swap(m_mapped, o.m_mapped);
        std::swap(m_mappedFlags, o.m_mappedFlags);
    }
}
