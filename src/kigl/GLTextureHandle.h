#pragma once

#include "kigl/kigl.h"

class GLTextureHandle {
public:
    GLTextureHandle()
    {}

    GLTextureHandle(int textureID)
        : m_textureID(textureID) {
    }

    GLTextureHandle(GLTextureHandle& handle) = delete;
    GLTextureHandle& operator=(GLTextureHandle& handle) = delete;

    GLTextureHandle(GLTextureHandle&& handle) noexcept
        : m_textureID(handle.m_textureID)
    {
        handle.m_textureID = 0;
    }

    GLTextureHandle& operator=(GLTextureHandle&& handle) noexcept
    {
        m_textureID = handle.m_textureID;
        handle.m_textureID = 0;
        return *this;
    }

    ~GLTextureHandle() {
        if (m_textureID > 0) {
            glDeleteTextures(1, &m_textureID);
        }
    }

    bool valid() { return m_textureID > 0;  }

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

    operator int() const { return m_textureID; }

public:
    int m_width{ 0 };
    int m_height{ 0 };

    GLuint m_textureID{ 0 };
};
