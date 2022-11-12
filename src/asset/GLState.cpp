#include "GLState.h"

GLState::GLState()
{
}

//void GLState::reload() {
//}

void GLState::track(GLenum key, bool initial) noexcept
{
    m_tracked.insert(key);
    if (initial) {
        enable(key);
    }
    else {
        disable(key);
    }
}

void GLState::enable(GLenum key) noexcept
{
    if (m_tracked.find(key) == m_tracked.end()) {
        glEnable(key);
        return;
    }

    if (m_enabled.find(key) != m_enabled.end()) {
        return;
    }
    glEnable(key);
    m_enabled.insert(key);
}

void GLState::disable(GLenum key) noexcept
{
    if (m_tracked.find(key) == m_tracked.end()) {
        glDisable(key);
        return;
    }

    if (m_enabled.find(key) == m_enabled.end()) {
        return;
    }
    glDisable(key);
    m_enabled.erase(key);
}

void GLState::cullFace(GLenum mode) noexcept
{
    if (m_cullFace != mode) {
        glCullFace(mode);
        m_cullFace = mode;
    }
}

void GLState::frontFace(GLenum mode) noexcept
{
    if (m_frontFace != mode) {
        glFrontFace(mode);
        m_frontFace = mode;
    }
}

void GLState::polygonFrontAndBack(GLenum mode) noexcept
{
    if (m_polygonFrontAndBack != mode) {
        glPolygonMode(GL_FRONT_AND_BACK, mode);
        m_polygonFrontAndBack = mode;
    }
}

void GLState::bindTextures(
    const GLuint unitIndexFirst,
    const std::vector<GLuint>& textureIDs) noexcept
{
    //glBindTextures(unitIndexFirst, textureIDs.size(), &textureIDs[0]);
    GLuint unitIndex = unitIndexFirst;
    for (auto textureID : textureIDs) {
        bindTexture(unitIndex++, textureID);
    }
}

void GLState::bindTexture(
    const GLuint unitIndex,
    const GLuint textureID) noexcept
{
    if (m_textureUnits[unitIndex] != textureID) {
        // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
        //KI_GL_CALL(glBindTextures(unitIndex, 1, &textureID));
        glBindTextureUnit(unitIndex, textureID);
        m_textureUnits[unitIndex] = textureID;
    }
}
