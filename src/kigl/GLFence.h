#pragma once

#include <fmt/format.h>

#include "kigl/kigl.h"

namespace kigl {
    struct GLFence {
        GLsync m_sync{ 0 };

        void setFence()
        {
            waitFence(false);
            m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        }

        // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
        void waitFence(bool debug)
        {
            if (!m_sync) return;

            size_t count = 0;
            GLenum res = GL_UNSIGNALED;
            while (res != GL_ALREADY_SIGNALED && res != GL_CONDITION_SATISFIED)
            {
                // 1 million == 1 ms
                res = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 2000000);
                count++;
            }

            if (debug) {
                if (count > 1) {
                    KI_OUT(fmt::format("[{}]", count));
                }
            }

            glDeleteSync(m_sync);
            m_sync = 0;
        }

        void waitFenceOnServer()
        {
            if (!m_sync) return;

            // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glWaitSync.xhtml
            glWaitSync(m_sync, 0, GL_TIMEOUT_IGNORED);
        }
    };
}
