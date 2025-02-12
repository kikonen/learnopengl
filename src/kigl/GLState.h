#pragma once

#include <set>
#include <unordered_map>
#include <vector>
#include <array>
#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "kigl/GLBlendMode.h"
#include "kigl/GLStencilMode.h"

namespace kigl {
    // NOTE KI GLState *MUST* be global singleton since there is only
    // single state in OpenGL (only single OpengL context used)
    // => Sharing it via Registry just complicates things redundantly
    // => If truly needed, can make ThreadLocal
    class GLState final
    {
    public:
        static GLState& get() noexcept;

        GLState();

        GLState& operator=(const GLState&) = delete;

        void invalidateAll();

        void track(GLenum key) noexcept;

        void setEnabled(GLenum key, bool enabled) noexcept;

        bool isEnabled(GLenum key) noexcept;

        void cullFace(GLenum mode) noexcept;
        void frontFace(GLenum mode) noexcept;

        void polygonFrontAndBack(GLenum mode) noexcept;

        void polygonOffset(const glm::vec2& offset) noexcept;

        void useProgram(GLuint programId) noexcept;
        void invalidateProgram() noexcept;

        void bindVAO(GLuint vaoId) noexcept;
        void invalidateVAO() noexcept;

        void bindTexture(
            const GLuint unitIndex,
            const GLuint textureID,
            bool force) noexcept;

        void unbindTexture(
            const GLuint unitIndex,
            bool force) noexcept;

        void invalidateTexture(
            const GLuint unitIndex) noexcept;

        int getFrameBuffer();

        // @return true if bind was done
        bool bindFrameBuffer(GLuint fbo, bool force) noexcept;
        void invalidateFrameBuffer();

        bool setViewport(const glm::vec4& viewport);
        void invalidateViewport();

        GLBlendMode setBlendMode(
            const GLBlendMode& mode);

        GLBlendMode setBlendMode(
            GLuint drawBuffer,
            const GLBlendMode& mode);

        void invalidateBlendMode();

        //void enableStencil(const GLStencilMode& mode);
        //void disableStencil();

        void setStencil(const GLStencilMode& mode);
        //void invalidateStencil();

        void setStencilOp(const GLStencilOp& op);
        void setStencilFunc(const GLStencilFunc& func);
        void setStencilMask(const GLStencilMask& mask);

        void setDepthFunc(const GLenum func);
        void setDepthMask(const GLenum mask);

        void setClearColor(const glm::vec4& clearColor);

        bool setBufferResolution(glm::vec2 bufferResolution);

    private:
        std::unordered_map<GLenum, int> m_enabled;

        std::array<GLuint, 128> m_textureUnits;

        int m_cullFace = -1;
        int m_frontFace = -1;
        int m_polygonFrontAndBack = -1;

        int m_programId = -1;
        int m_vaoId = -1;

        int m_fbo = -1;

        glm::vec4 m_viewport{ 0.f };

        GLBlendMode m_blendMode;
        std::array<GLBlendMode, 4> m_blendModes;

        GLStencilOp m_stencilOp;
        GLStencilFunc m_stencilFunc;
        GLStencilMask m_stencilMask;

        GLenum m_depthFunc = -1;
        GLenum m_depthMask = -1;

        glm::vec4 m_clearColor{ 0.f };

        glm::vec2 m_bufferResolution{ 0.f };
    };
}
