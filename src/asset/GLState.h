#pragma once

#include <set>

#include "ki/GL.h"

class GLState final
{
public:
    GLState();

    void reload();

    void enable(GLenum key);
    void disable(GLenum key);

    void cullFace(GLenum mode);
    void frontFace(GLenum mode);

    void polygonMode(GLenum face, GLenum mode);

private:
    std::set<GLenum> enabled;
    std::set<GLenum> trackedEnabled;
};

