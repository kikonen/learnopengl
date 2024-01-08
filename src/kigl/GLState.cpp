#include "GLState.h"

namespace kigl {
    GLState::GLState()
    {
        clear();
    }

    void GLState::clear() {
        for (auto& it : m_enabled) {
            it.second = -1;
        }

        for (int i = 0; i < m_textureUnits.size(); i++) {
            m_textureUnits[i] = 0;
        }

        m_cullFace = -1;
        m_frontFace = -1;
        m_polygonFrontAndBack = -1;

        m_programId = -1;
        m_vaoId = -1;

        m_fbo = -1;

        m_viewport = { 0.f, 0.f, 0.f, 0.f };

        m_blendMode = { 0, 0, 0, 0, 0 };

        m_stencilOp = { 0, 0, 0 };
        m_stencilFunc = { 0, 0, 0 };
        m_stencilMask = { 0xffff };

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
        if (const auto& it = m_enabled.find(key);
            it != m_enabled.end())
        {
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

        const auto curr = m_textureUnits[unitIndex];
        const bool changed = force || curr != textureID;
        if (!changed) return;

        // NOTE KI logic failing when new texture generated, but its' ID does not change
        // (i.e. IDs are apparently reused after texture delete)
        // => caused viewport diappear after resising main viewport
        {
            // https://computergraphics.stackexchange.com/questions/4479/how-to-do-texturing-with-opengl-direct-state-access
            //KI_GL_CALL(glBindTextures(unitIndex, 1, &textureID));
            //KI_DEBUG(fmt::format("BIND_TEXTURE: unitIndex={}, textureID={}", unitIndex, textureID));
            glBindTextureUnit(unitIndex, textureID);
            m_textureUnits[unitIndex] = textureID;
        }
    }

    void GLState::unbindTexture(
        const GLuint unitIndex,
        bool force) noexcept
    {
        bindTexture(unitIndex, 0, force);
    }

    int GLState::getFrameBuffer()
    {
        // https://stackoverflow.com/questions/27459859/how-to-check-which-frame-buffer-object-is-currently-bound-in-opengl
        GLint drawFboId;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);

        if (m_fbo >= 0) {
            if (m_fbo != drawFboId) {
                throw std::runtime_error{ fmt::format("CONTEXT: FBO mismatch: expectedFbo={}, actualFbo={}", m_fbo, drawFboId) };
            }
            return m_fbo;
        }

        return drawFboId;
    }

    bool GLState::bindFrameBuffer(GLuint fbo, bool force) noexcept
    {
        if (m_fbo != fbo || force) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
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
            glm::uvec4 vp{ viewport };
            glViewport(vp[0], vp[1], vp[2], vp[3]);
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
        GLBlendMode old = m_blendMode;

        if (m_blendMode != mode)
        {
            m_blendMode = mode;
            m_blendMode.apply();
        }

        return old;
    }

    void GLState::invalidateBlendMode()
    {
        m_blendMode = { 0, 0, 0, 0 };
    }

    //void GLState::enableStencil(const GLStencilMode& mode)
    //{
    //    setStencilMode(mode);
    //    setEnabled(GL_STENCIL_TEST, true);
    //}
    //
    //void GLState::disableStencil()
    //{
    //    setEnabled(GL_STENCIL_TEST, false);
    //
    //    // NOTE KI *mask* affects clear
    //    // https://community.khronos.org/t/how-to-clear-stencil-buffer-after-stencil-test/15882/2
    //    //setStencilMode({});
    //    invalidateBlendMode();
    //    glStencilMask(0xff);
    //}

    void GLState::setStencil(const GLStencilMode& mode)
    {
        setStencilOp(mode.op);
        setStencilFunc(mode.func);
        setStencilMask(mode.mask);
        setEnabled(GL_STENCIL_TEST, mode.testEnabled);
    }

    void GLState::setStencilOp(const GLStencilOp& op) {
        if (m_stencilOp != op)
        {
            m_stencilOp = op;
            glStencilOp(op.sfail, op.dpfail, op.dppass);
        }
    }

    void GLState::setStencilFunc(const GLStencilFunc& func) {
        if (m_stencilFunc != func)
        {
            m_stencilFunc = func;
            glStencilFunc(func.func, func.ref, func.mask);
        }
    }

    void GLState::setStencilMask(const GLStencilMask& mask) {
        if (m_stencilMask != mask)
        {
            m_stencilMask = mask;
            glStencilMask(mask.mask);
        }
    }

    //void GLState::invalidateStencil()
    //{
    //    //m_stencilMode = { 0, 0, 0, 0, 0, 0, 0 };
    //}

    void GLState::setDepthFunc(const GLenum func)
    {
        if (m_depthFunc != func) {
            m_depthFunc = func;
            glDepthFunc(func);
        }
    }

    void GLState::setDepthMask(const GLenum mask)
    {
        if (m_depthMask != mask) {
            m_depthMask = mask;
            glDepthMask(mask);
        }
    }

    void GLState::clearColor(const glm::vec4& clearColor)
    {
        if (m_clearColor != clearColor) {
            m_clearColor = clearColor;
            glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        }
    }

    bool GLState::setBufferResolution(glm::vec2 bufferResolution)
    {
        bool changed = m_bufferResolution != bufferResolution;
        if (changed) {
            m_bufferResolution = bufferResolution;
        }
        return changed;
    }
}
