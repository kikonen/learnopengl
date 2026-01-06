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

        GLTextureHandle(int textureID, const glm::ivec2& size)
            : m_textureID(textureID),
            m_width{ size.x },
            m_height{ size.y }
        {
        }

        GLTextureHandle(GLTextureHandle& o) = delete;
        GLTextureHandle& operator=(GLTextureHandle& o) = delete;

        GLTextureHandle(GLTextureHandle&& o) noexcept
        {
            swap(o);
        }

        GLTextureHandle& operator=(GLTextureHandle&& o) noexcept
        {
            GLTextureHandle tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        ~GLTextureHandle()
        {
            release();
        }

        void swap(GLTextureHandle& o) noexcept;

        bool valid() const noexcept
        {
            return m_textureID > 0;
        }

        glm::ivec2 getSize() const noexcept
        {
            return { m_width, m_height };
        }

        void setSize(const glm::ivec2& size)
        {
            m_width = size.x;
            m_height = size.y;
        }

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

        void release()
        {
            if (m_textureID > 0)
            {
                glDeleteTextures(1, &m_textureID);
                m_textureID = 0;
            }
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
