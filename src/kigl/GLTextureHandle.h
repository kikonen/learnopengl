#pragma once

#include "ki/GL.h"

class GLTextureHandle {
public:
    GLTextureHandle()
    {}

    GLTextureHandle(int textureID)
        : m_textureID(textureID) {
    }

    bool valid() { return m_textureID > 0;  }

    void create(
            const std::string& name,
            GLenum target,
            int width,
            int height
        )
    {
        if (m_textureID > 0) return;
        glCreateTextures(target, 1, &m_textureID);

        m_width = width;
        m_height = height;

        glObjectLabel(GL_TEXTURE, m_textureID, name.length(), name.c_str());
    }

    ~GLTextureHandle() {
        if (m_textureID > 0) {
            glDeleteTextures(1, &m_textureID);
        }
    }

    operator int() const { return m_textureID; }

public:
    int m_width{ 0 };
    int m_height{ 0 };

    GLuint m_textureID{ 0 };
};
