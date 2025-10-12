#include "CubeMapDebugTexture.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/ScreenTri.h"

#include "registry/Registry.h"

#include "CubeRender.h"

namespace {
    inline const std::string SHADER_FLAT_CUBE_MAP{ "flat_cube_map" };
}

namespace render
{
    CubeMapDebugTexture::CubeMapDebugTexture(
        const std::string& name)
        : m_name{ name }
    { 
    }

    CubeMapDebugTexture::~CubeMapDebugTexture() = default;

    void CubeMapDebugTexture::prepare(int cubeSize)
    {
        if (cubeSize == m_cubeSize) return;
        m_cubeSize = cubeSize;

        const glm::ivec2 flatSize{ m_cubeSize * 4 * 0.25f, m_cubeSize * 3 * 0.25f };
        {
            m_handle.release();
            m_handle.create(fmt::format("flat_cube_{}", m_name), GL_TEXTURE_2D, flatSize.x, flatSize.y);

            // NOTE KI allow HDR
            glTextureStorage2D(m_handle, 1, GL_RGB16F, flatSize.x, flatSize.y);

            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }

    void CubeMapDebugTexture::render(
        const kigl::GLTextureHandle& cubeHandle,
        bool equirectangular)
    {
        auto& state = kigl::GLState::get();
        state.frontFace(GL_CCW);

        {
            auto* program = Program::get(ProgramRegistry::get().getProgram(SHADER_FLAT_CUBE_MAP));

            program->prepareRT();
            program->bind();
            program->setBool("u_equirectangular", equirectangular);
        }

        const glm::ivec2 flatSize = m_handle.getSize();

        std::unique_ptr<FrameBuffer> captureFBO{ nullptr };
        {
            auto buffer = new FrameBuffer(
                "flat_capture_fbo",
                {
                    flatSize.x, flatSize.y,
                    {
                        FrameBufferAttachment::getDrawBuffer(),
                        // NOTE KI skip depth, not relevant in this case
                        FrameBufferAttachment::getDepthRbo(),
                    }
                });
            captureFBO.reset(buffer);
            captureFBO->prepare();
        }

        glViewport(0, 0, flatSize.x, flatSize.y);
        glNamedFramebufferTexture(*captureFBO, GL_COLOR_ATTACHMENT0, m_handle, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *captureFBO);

        {
            const glm::vec4 clearColor{ 0.f, 0.f, 0.f, 0.f };
            const float clearDepth{ 1.f };
            glClearNamedFramebufferfv(*captureFBO, GL_COLOR, 0, glm::value_ptr(clearColor));
            glClearNamedFramebufferfv(*captureFBO, GL_DEPTH, 0, &clearDepth);
        }

        state.bindTexture(UNIT_EDITOR_CUBE_MAP, cubeHandle, false);

        render::ScreenTri tri;
        tri.draw();

        state.unbindTexture(UNIT_EDITOR_CUBE_MAP, cubeHandle);
    }
}
