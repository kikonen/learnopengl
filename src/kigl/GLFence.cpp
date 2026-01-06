#include "GLFence.h"

#include <fmt/format.h>

namespace {
    constexpr long WAIT_DELAY_MS = 5;
    constexpr long WAIT_DELAY = WAIT_DELAY_MS * 1000 * 1000;
}

namespace kigl {
    GLFence::GLFence(std::string_view name)
        : m_name(name)
    {}

    GLFence::~GLFence()
    {
        release();
    }

    void GLFence::swap(GLFence& o) noexcept
    {
        std::swap(m_sync, o.m_sync);
        std::swap(m_name, o.m_name);
    }

    void GLFence::release()
    {
        if (m_sync) {
            glDeleteSync(m_sync);
            m_sync = 0;
        }
    }

    void GLFence::setFence()
    {
        waitFence();
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    bool GLFence::setFenceIfNotSet()
    {
        if (isSet()) return false;
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        return true;
    }

    void GLFence::waitFence()
    {
        if (!m_sync) return;

        glClientWaitSync(
            m_sync,
            GL_SYNC_FLUSH_COMMANDS_BIT,
            GL_TIMEOUT_IGNORED);

        release();
    }

    void GLFence::waitFenceOnServer() const noexcept
    {
        if (!m_sync) return;

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glWaitSync.xhtml
        glWaitSync(m_sync, 0, GL_TIMEOUT_IGNORED);
    }
}
