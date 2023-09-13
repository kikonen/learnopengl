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

    void create(const std::string& name, GLenum target) {
        if (m_textureID > 0) return;
        glCreateTextures(target, 1, &m_textureID);

        glObjectLabel(GL_TEXTURE, m_textureID, name.length(), name.c_str());
    }

    ~GLTextureHandle() {
        if (m_textureID > 0) {
            glDeleteTextures(1, &m_textureID);
        }
    }

    operator int() const { return m_textureID; }

public:
    GLuint m_textureID{ 0 };
};
