#pragma once

#include <set>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "kigl/GLBlendMode.h"
#include "kigl/GLStencilMode.h"

class GLState final
{
public:
    GLState();

    void clear();

    void track(GLenum key) noexcept;

    void setEnabled(GLenum key, bool enabled) noexcept;

    bool isEnabled(GLenum key) noexcept;

    void cullFace(GLenum mode) noexcept;
    void frontFace(GLenum mode) noexcept;

    void polygonFrontAndBack(GLenum mode) noexcept;

    void polygonOffset(const glm::vec2& offset) noexcept;

    void useProgram(GLuint programId) noexcept;
    void bindVAO(GLuint vaoId) noexcept;

    void bindTexture(
        const GLuint unitIndex,
        const GLuint textureID,
        bool force) noexcept;

    int getFrameBuffer();

    // @return true if bind was done
    bool bindFrameBuffer(GLuint fbo, bool force) noexcept;
    void clearFrameBuffer();

    bool setViewport(const glm::vec4& viewport);
    void clearViewport();

    GLBlendMode setBlendMode(const GLBlendMode& mode);
    void invalidateBlendMode();

    void enableStencil(const GLStencilMode& mode);
    void disableStencil();

    void setStencilMode(const GLStencilMode& mode);
    void invalidateStencilMode();

    void setDepthFunc(const GLenum func);
    void setDepthMask(const GLenum mask);

    void clearColor(const glm::vec4& clearColor);

    bool setBufferResolution(glm::vec2 bufferResolution);

private:
    std::unordered_map<GLenum, int> m_enabled;

    std::unordered_map<GLuint, GLuint> m_textureUnits;

    int m_cullFace = -1;
    int m_frontFace = -1;
    int m_polygonFrontAndBack = -1;

    int m_programId = -1;
    int m_vaoId = -1;

    int m_fbo = -1;

    glm::vec4 m_viewport{ 0.f };

    GLBlendMode m_blendMode{ 0, 0, 0, 0, 0 };

    GLStencilMode m_stencilMode{};

    GLenum m_depthFunc = -1;
    GLenum m_depthMask = -1;

    glm::vec4 m_clearColor{ 0.f };

    glm::vec2 m_bufferResolution{ 0.f };
};

