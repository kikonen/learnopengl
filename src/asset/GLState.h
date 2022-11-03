#pragma once

#include <set>
#include <map>
#include <vector>

#include "ki/GL.h"

class GLState final
{
public:
    GLState();

    //void reload();

    void track(GLenum key, bool initial) noexcept;

    inline void enable(GLenum key) noexcept;
    inline void disable(GLenum key) noexcept;

    void cullFace(GLenum mode) noexcept;
    void frontFace(GLenum mode) noexcept;

    void polygonFrontAndBack(GLenum mode) noexcept;

    void bindTextures(
        const GLuint unitIndexFirst,
        const std::vector<GLuint>& textureIDs) noexcept;

    void bindTexture(
        const GLuint unitIndex,
        const GLuint textureID) noexcept;

private:
    std::set<GLenum> enabled;
    std::set<GLenum> tracked;

    std::map<GLuint, GLuint> textureUnits;

    int m_cullFace = -1;
    int m_frontFace = -1;
    int m_polygonFrontAndBack = -1;
};

