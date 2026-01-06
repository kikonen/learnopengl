#include "GLTextureHandle.h"

#include <algorithm>

namespace kigl
{
    void GLTextureHandle::swap(GLTextureHandle& o) noexcept
    {
        std::swap(m_textureID, o.m_textureID);
        std::swap(m_width, o.m_width);
        std::swap(m_height, o.m_height);
    }
}
