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
        discard();
    }

    void GLFence::swap(GLFence& o) noexcept
    {
        std::swap(m_sync, o.m_sync);
        std::swap(m_name, o.m_name);
    }

    void GLFence::discard()
    {
        if (m_sync) {
            glDeleteSync(m_sync);
            m_sync = 0;
        }
    }

    void GLFence::setFence(bool debug)
    {
        waitFence(debug);
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    bool GLFence::setFenceIfNotSet(bool debug)
    {
        if (isSet()) return false;
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        return true;
    }

    void GLFence::waitFence(bool debug)
    {
        if (!m_sync) return;

        size_t count = 0;
        GLenum res = GL_UNSIGNALED;
        while (res != GL_ALREADY_SIGNALED && res != GL_CONDITION_SATISFIED)
        {
            // 1 million == 1 ms
            res = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, WAIT_DELAY);
            //res = glClientWaitSync(m_sync, 0, 2000000);
            count++;
        }

        if (debug) {
            if (count > 1) {
                KI_OUT(fmt::format("[{}={}-{}ms]", m_name, count, WAIT_DELAY_MS * count));
            }
        }

        discard();
    }

    void GLFence::waitFenceOnServer() const noexcept
    {
        if (!m_sync) return;

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glWaitSync.xhtml
        glWaitSync(m_sync, 0, GL_TIMEOUT_IGNORED);
    }
}
