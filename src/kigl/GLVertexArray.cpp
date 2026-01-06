#include "GLVertexArray.h"

#include <algorithm>

namespace kigl
{
    void GLVertexArray::swap(GLVertexArray& o) noexcept
    {
        std::swap(m_id, o.m_id);
        std::swap(m_created, o.m_created);
    }
}
