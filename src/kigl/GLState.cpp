#include "GLState.h"

GLState::GLState()
{
    clear();
}

void GLState::clear() {
    for (auto& it : m_enabled) {
        it.second = -1;
    }

    m_textureUnits.clear();

    m_cullFace = -1;
    m_frontFace = -1;
    m_polygonFrontAndBack = -1;

    m_programId = -1;
    m_vaoId = -1;

    m_fbo = -1;

    m_viewport = { 0.f, 0.f, 0.f, 0.f };

    m_blendMode = { 0, 0, 0, 0, 0 };

    m_depthFunc = -1;
    m_depthMask = -1;

    m_clearColor = { 0.f, 0.f, 0.f, 0.f };
}


void GLState::track(GLenum key) noexcept
{
    m_enabled[key] = -1;
}

void GLState::setEnabled(GLenum key, bool enabled) noexcept
{
    int value = enabled;
    const auto& it = m_enabled.find(key);
    const bool changed = it == m_enabled.end() || it->second != value;
    if (!changed) return;

    if (enabled) {
        glEnable(key);
    }
    else {
        glDisable(key);
    }

    if (it != m_enabled.end()) {
        it->second = value;
    }
}

bool GLState::isEnabled(GLenum key) noexcept
{
    const auto& it = m_enabled.find(key);
    if (it != m_enabled.end()) {
        return it->second == 1;
    }
    return false;
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

    const auto& it = m_enabled.find(unitIndex);
    const bool changed = force || it == m_enabled.end() || it->second != textureID;
    if (!changed) return;

    // NOTE KI logic failing when new texture generated, but its' ID does not change
    // (i.e. IDs are apparently reused after texture delete)
    // => caused viewport diappear after resising main viewport
    if (changed) {
        // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
        //KI_GL_CALL(glBindTextures(unitIndex, 1, &textureID));
        //KI_DEBUG(fmt::format("BIND_TEXTURE: unitIndex={}, textureID={}", unitIndex, textureID));
        glBindTextureUnit(unitIndex, textureID);
        m_textureUnits[unitIndex] = textureID;
    }
}

bool GLState::bindFrameBuffer(GLuint fbo, bool force) noexcept
{
    if (m_fbo != fbo || force) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        m_fbo = fbo;
        return true;
    }
    return false;
}

void GLState::clearFrameBuffer() {
    m_fbo = -1;
}

bool GLState::setViewport(const glm::vec4& viewport)
{
    if (m_viewport != viewport) {
        m_viewport = viewport;
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        return true;
    }
    return false;
}

void GLState::clearViewport()
{
    m_viewport = { 0.f, 0.f, 0.f, 0.f };
}

GLBlendMode GLState::setBlendMode(const GLBlendMode& mode)
{
    GLBlendMode old= m_blendMode;

    if (m_blendMode != mode)
    {
        m_blendMode = mode;

        glBlendEquation(m_blendMode.blendEquation);

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

void GLState::clearColor(const glm::vec4& clearColor)
{
    if (m_clearColor != clearColor) {
        m_clearColor = clearColor;
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    }
}

