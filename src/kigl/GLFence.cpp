#include "GLFence.h"

#include <fmt/format.h>

namespace {
    constexpr long WAIT_DELAY = 5 * 1000 *1000;
}

namespace kigl {
    GLFence::GLFence(std::string_view name)
        : m_name(name)
    {}

    void GLFence::setFence(bool debug)
    {
        waitFence(debug);
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
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
                KI_OUT(fmt::format("[{}={}]", m_name,count));
            }
        }

        glDeleteSync(m_sync);
        m_sync = 0;
    }

    void GLFence::waitFenceOnServer() const noexcept
    {
        if (!m_sync) return;

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glWaitSync.xhtml
        glWaitSync(m_sync, 0, GL_TIMEOUT_IGNORED);
    }
}
