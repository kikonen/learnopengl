#pragma once

#include <set>

#include "ki/GL.h"

class GLState final
{
public:
    GLState();

    //void reload();

    void track(GLenum key, bool initial);

    inline void enable(GLenum key);
    inline void disable(GLenum key);

    void cullFace(GLenum mode);
    void frontFace(GLenum mode);

    void polygonFrontAndBack(GLenum mode);

private:
    std::set<GLenum> enabled;
    std::set<GLenum> tracked;

    int m_cullFace = -1;
    int m_frontFace = -1;
    int m_polygonFrontAndBack = -1;
};

