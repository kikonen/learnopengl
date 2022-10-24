#include "GLState.h"

GLState::GLState()
{
}

//void GLState::reload() {
//}

void GLState::track(GLenum key, bool initial)
{
    tracked.insert(key);
    if (initial) {
        enable(key);
    }
    else {
        disable(key);
    }
}

void GLState::enable(GLenum key)
{
    if (tracked.find(key) == tracked.end()) {
        glEnable(key);
        return;
    }

    if (enabled.find(key) != enabled.end()) {
        return;
    }
    glEnable(key);
    enabled.insert(key);
}

void GLState::disable(GLenum key)
{
    if (tracked.find(key) == tracked.end()) {
        glDisable(key);
        return;
    }

    if (enabled.find(key) == enabled.end()) {
        return;
    }
    glDisable(key);
    enabled.erase(key);
}

void GLState::cullFace(GLenum mode)
{
    if (m_cullFace != mode) {
        glCullFace(mode);
        m_cullFace = mode;
    }
}

void GLState::frontFace(GLenum mode)
{
    if (m_frontFace != mode) {
        glFrontFace(mode);
        m_frontFace = mode;
    }
}

void GLState::polygonFrontAndBack(GLenum mode)
{
    if (m_polygonFrontAndBack != mode) {
        glPolygonMode(GL_FRONT_AND_BACK, mode);
        m_polygonFrontAndBack = mode;
    }
}

void GLState::bindTextures(
    const GLuint unitIndexFirst,
    const std::vector<GLuint>& textureIDs)
{
    //glBindTextures(unitIndexFirst, textureIDs.size(), &textureIDs[0]);
    GLuint unitIndex = unitIndexFirst;
    for (auto textureID : textureIDs) {
        bindTexture(unitIndex++, textureID);
    }
}

void GLState::bindTexture(
    const GLuint unitIndex,
    const GLuint textureID)
{
    if (textureUnits[unitIndex] != textureID) {
        KI_GL_CALL(glBindTextures(unitIndex, 1, &textureID));
        textureUnits[unitIndex] = textureID;
    }
}
