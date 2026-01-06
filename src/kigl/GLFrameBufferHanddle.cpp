#include "GLFrameBufferHandle.h"

#include <algorithm>

namespace kigl {
    void GLFrameBufferHandle::swap(GLFrameBufferHandle& o) noexcept
    {
        std::swap(m_fbo, o.m_fbo);
    }
}
