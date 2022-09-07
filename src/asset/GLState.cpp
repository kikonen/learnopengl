#include "GLState.h"

GLState::GLState()
{
}

void GLState::reload() {
}

void GLState::enable(GLenum key)
{
    //if (enabled.find(key) != enabled.end()) return;
    glEnable(key);
    enabled.insert(key);
}

void GLState::disable(GLenum key)
{
    //if (enabled.find(key) == enabled.end()) return;
    glDisable(key);
    enabled.erase(key);
}

void GLState::cullFace(GLenum mode)
{
    glCullFace(mode);
}

void GLState::frontFace(GLenum mode)
{
    glFrontFace(mode); 
}

void GLState::polygonMode(GLenum face, GLenum mode)
{
    glPolygonMode(face, mode);
}
