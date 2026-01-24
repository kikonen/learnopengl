#include "GLFence.h"

#include <fmt/format.h>

#include "util/Log.h"

namespace {
    constexpr long WAIT_DELAY_MS = 10;
    constexpr long WAIT_DELAY = WAIT_DELAY_MS * 1000 * 1000;

    // NOTE KI warn if wait exceeds this threshold, but keep waiting
    constexpr long WARN_WAIT_COUNT = 10;
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

        int count = 0;
        GLenum res = GL_UNSIGNALED;
        bool warned = false;

        // NOTE KI must wait until signaled - giving up causes GPU/CPU race conditions
        // that manifest as flickering/disappearing meshes
        while (res != GL_ALREADY_SIGNALED && res != GL_CONDITION_SATISFIED)
        {
            // 1 million == 1 ms
            res = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, WAIT_DELAY);
            count++;

            if (!warned && count >= WARN_WAIT_COUNT) {
                KI_WARN(fmt::format("GLFence '{}': long wait - {}ms so far", m_name, count * WAIT_DELAY_MS));
                warned = true;
            }

            // NOTE KI handle wait failure - should not happen in normal operation
            if (res == GL_WAIT_FAILED) {
                KI_ERROR(fmt::format("GLFence '{}': wait failed!", m_name));
                break;
            }
        }

        release();
    }

    void GLFence::waitFenceOnServer() const noexcept
    {
        if (!m_sync) return;

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glWaitSync.xhtml
        glWaitSync(m_sync, 0, GL_TIMEOUT_IGNORED);
    }
}
