#pragma once

#include "kigl/kigl.h"
#include "kigl/GLState.h"

namespace kigl {
    class GLTextureHandle {
    public:
        GLTextureHandle()
        {}

        GLTextureHandle(int textureID)
            : m_textureID(textureID) {
        }

        GLTextureHandle(GLTextureHandle& o) = delete;
        GLTextureHandle& operator=(GLTextureHandle& o) = delete;

        GLTextureHandle(GLTextureHandle&& o) noexcept
            : m_textureID{ o.m_textureID }
        {
            o.m_textureID = 0;
        }

        GLTextureHandle& operator=(GLTextureHandle&& o) noexcept
        {
            m_textureID = o.m_textureID;
            o.m_textureID = 0;
            return *this;
        }

        ~GLTextureHandle() {
            if (m_textureID > 0) {
                glDeleteTextures(1, &m_textureID);
            }
        }

        bool valid() { return m_textureID > 0; }

        void create(
            std::string_view name,
            GLenum target,
            int width,
            int height
        )
        {
            if (m_textureID > 0) return;
            glCreateTextures(target, 1, &m_textureID);

            m_width = width;
            m_height = height;

            kigl::setLabel(GL_TEXTURE, m_textureID, name);
        }

        void bindTexture(int unitIndex)
        {
            kigl::GLState::get().bindTexture(unitIndex, m_textureID, false);
        }

        void unbindTexture(int unitIndex)
        {
            kigl::GLState::get().unbindTexture(unitIndex, false);
        }

        operator int() const { return m_textureID; }

    public:
        int m_width{ 0 };
        int m_height{ 0 };

        GLuint m_textureID{ 0 };
    };
}
