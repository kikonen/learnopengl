#include "EnvironmentMap.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "engine/PrepareContext.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/ScreenTri.h"

#include "registry/Registry.h"

#include "CubeRender.h"

namespace {
    inline const std::string SHADER_HDRI_CUBE_MAP{ "hdri_cube_map" };
    inline const std::string SHADER_FLAT_CUBE_MAP{ "flat_cube_map" };
}

namespace render {
    void EnvironmentMap::prepareRT(
        const PrepareContext& ctx,
        int size)
    {
        const auto& assets = ctx.getAssets();
        auto* registry = ctx.getRegistry();
        auto& state = kigl::GLState::get();

        if (m_hdriTextureID <= 0) return;

        m_size = size;

        {
            m_cubeTexture.create(fmt::format("{}_env_map", m_name), GL_TEXTURE_CUBE_MAP, m_size, m_size);

            glTextureStorage2D(m_cubeTexture, 1, GL_RGB16F, m_size, m_size);

            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        {
            auto* program = Program::get(ProgramRegistry::get().getProgram(SHADER_HDRI_CUBE_MAP));

            program->prepareRT();
            program->bind();
            state.bindTexture(UNIT_HDR_TEXTURE, m_hdriTextureID, false);

            CubeRender renderer;
            renderer.render(program, m_cubeTexture, m_size);
            state.unbindTexture(UNIT_HDR_TEXTURE, false);
            state.invalidateAll();
        }

        //const glm::ivec2 flatSize{ m_size * 4 * 0.25f, m_size * 3 * 0.25f };
        //{
        //    m_flatTexture.create(fmt::format("{}_env_flat", m_name), GL_TEXTURE_2D, flatSize.x, flatSize.y);

        //    glTextureStorage2D(m_flatTexture, 1, GL_RGB16F, flatSize.x, flatSize.y);

        //    glTextureParameteri(m_flatTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //    glTextureParameteri(m_flatTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //    glTextureParameteri(m_flatTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //    glTextureParameteri(m_flatTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        //    glTextureParameteri(m_flatTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //}
        {
            auto& state = kigl::GLState::get();

            // NTOE KI cube drawn from inside-out
            state.frontFace(GL_CCW);

            auto* program = Program::get(ProgramRegistry::get().getProgram(SHADER_FLAT_CUBE_MAP));

            program->prepareRT();
            program->bind();
            state.bindTexture(UNIT_EDITOR_CUBE_MAP, m_cubeTexture, false);

            const glm::ivec2 flatSize{ m_size * 4 * 0.25f, m_size * 3 * 0.25f };

            std::unique_ptr<render::FrameBuffer> captureFBO{ nullptr };
            {
                auto buffer = new render::FrameBuffer(
                    "flat_capture_fbo",
                    {
                        flatSize.x, flatSize.y,
                        {

                            //render::FrameBufferAttachment::getDrawBuffer(),
                            render::FrameBufferAttachment::getTextureRGBA(GL_COLOR_ATTACHMENT0),
                            render::FrameBufferAttachment::getDepthRbo(),
                        }
                    });
                captureFBO.reset(buffer);
                captureFBO->prepare();
            }

            glViewport(0, 0, flatSize.x, flatSize.y);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *captureFBO);
            captureFBO->clearAll();

            render::ScreenTri tri;
            tri.draw();

            auto& att = captureFBO->m_spec.attachments[0];
            m_flatTexture = att.textureID;
            m_flatTexture.setSize(flatSize);
            att.textureID = 0;
            att.createdTexture = false;

            state.unbindTexture(UNIT_EDITOR_CUBE_MAP, m_cubeTexture);
        }
    }

    void EnvironmentMap::bindTexture(
        kigl::GLState& state,
        int unitIndex)
    {
        state.bindTexture(unitIndex, m_cubeTexture, false);
    }
}
