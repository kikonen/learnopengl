#include "GLState.h"

GLState::GLState()
{
}

//void GLState::reload() {
//}

void GLState::track(GLenum key, bool initial) noexcept
{
    setEnabled(key, initial);
}

void GLState::setEnabled(GLenum key, bool enabled) noexcept
{
    const auto& it = m_enabled.find(key);
    const bool changed = it == m_enabled.end() || it->second != enabled;
    if (!changed) return;

    if (enabled) {
        glEnable(key);
    }
    else {
        glDisable(key);
    }

    if (it != m_enabled.end()) {
        it->second = enabled;
    }
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

void GLState::polygonOffset(const glm::vec2& offset) noexcept
{
    glPolygonOffset(offset[0], offset[1]);
}

void GLState::useProgram(GLuint programId) noexcept
{
    if (m_programId != programId) {
        //KI_GL_CALL(glUseProgram(m_programId));
        glUseProgram(programId);
        m_programId = programId;
    }
}

void GLState::bindVAO(GLuint vaoId) noexcept
{
    if (m_vaoId != vaoId) {
        glBindVertexArray(vaoId);
        m_vaoId = vaoId;
    }
}

void GLState::bindTexture(
    const GLuint unitIndex,
    const GLuint textureID,
    bool force) noexcept
{
    //force = true;

    // NOTE KI logic failing when new texture generated, but its' ID does not change
    // (i.e. IDs are apparently reused after texture delete)
    // => caused viewport diappear after resising main viewport
    if (force || m_textureUnits[unitIndex] != textureID) {
        // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
        //KI_GL_CALL(glBindTextures(unitIndex, 1, &textureID));
        //KI_DEBUG(fmt::format("BIND_TEXTURE: unitIndex={}, textureID={}", unitIndex, textureID));
        glBindTextureUnit(unitIndex, textureID);
        m_textureUnits[unitIndex] = textureID;
    }
}

void GLState::bindFrameBuffer(GLuint fbo, bool force) noexcept
{
    if (m_fbo != fbo || force) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        m_fbo = fbo;
    }
}

GLBlendMode GLState::setBlendMode(const GLBlendMode& mode)
{
    GLBlendMode old= m_blendMode;

    if (m_blendMode != mode)
    {
        m_blendMode = mode;

        // NOTE KI FrameBufferAttachment::getTextureRGB() also fixes this
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFuncSeparate(mode.srcRGB, mode.dstRGB, mode.srcAlpha, mode.dstAlpha);
    }

    return old;
}

void GLState::clearBlendMode()
{
    m_blendMode = { 0, 0, 0, 0 };
}

GLenum GLState::setDepthFunc(const GLenum func)
{
    GLenum old = m_depthFunc;

    if (m_depthFunc != func) {
        m_depthFunc = func;
        glDepthFunc(func);
    }

    return old;
}

GLenum GLState::setDepthMask(const GLenum mask)
{
    GLenum old = m_depthMask;

    if (m_depthMask != mask) {
        m_depthMask = mask;
        glDepthMask(mask);
    }

    return old;
}
