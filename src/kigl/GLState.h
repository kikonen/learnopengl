#pragma once

#include <set>
#include <unordered_map>
#include <vector>

#include "ki/GL.h"

#include "kigl/GLBlendMode.h"

class GLState final
{
public:
    GLState();

    //void reload();

    void track(GLenum key, bool initial) noexcept;

    void setEnabled(GLenum key, bool enabled) noexcept;

    void cullFace(GLenum mode) noexcept;
    void frontFace(GLenum mode) noexcept;

    void polygonFrontAndBack(GLenum mode) noexcept;

    void useProgram(GLuint programId) noexcept;
    void bindVAO(GLuint vaoId) noexcept;

    void bindTexture(
        const GLuint unitIndex,
        const GLuint textureID,
        bool force) noexcept;

    void bindFrameBuffer(GLuint fbo, bool force) noexcept;

    void setBlendMode(const GLBlendMode& mode);
    void setDepthFunc(const GLenum mode);

private:
    std::unordered_map<GLenum, bool> m_enabled;

    std::unordered_map<GLuint, GLuint> m_textureUnits;

    int m_cullFace = -1;
    int m_frontFace = -1;
    int m_polygonFrontAndBack = -1;

    int m_programId = -1;
    int m_vaoId = -1;

    int m_fbo = -1;

    GLBlendMode m_blendMode{ 0, 0, 0, 0 };
    GLenum m_depthFunc = -1;
};

