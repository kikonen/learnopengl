#include "PrefilterMap.h"

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLFrameBufferHandle.h"
#include "kigl/GLRenderBufferHandle.h"
#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "engine/PrepareContext.h"

#include "render/TextureCube.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/ScreenTri.h"

#include "registry/Registry.h"


namespace {
    inline const std::string SHADER_PREFILTER_CUBE_MAP{ "prefilter_cube_map" };
    inline const std::string SHADER_FLAT_CUBE_MAP{ "flat_cube_map" };

    constexpr unsigned int MAX_MIP_LEVELS = 5;
}

namespace render {
    void PrefilterMap::prepareRT(
        const PrepareContext& ctx)
    {
        const auto& assets = ctx.getAssets();
        auto* registry = ctx.getRegistry();
        auto& state = kigl::GLState::get();

        if (m_envCubeMapID <= 0) return;

        m_size = assets.prefilterMapSize;

        {
            m_cubeTexture.create("prefilter_map", GL_TEXTURE_CUBE_MAP, m_size, m_size);

            // https://www.khronos.org/opengl/wiki/Common_Mistakes#Creating_a_complete_texture
            glTextureStorage2D(m_cubeTexture, MAX_MIP_LEVELS, GL_RGB16F, m_size, m_size);

            // https://stackoverflow.com/questions/37232110/opengl-cubemap-writing-to-mipmap
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_BASE_LEVEL, 0);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAX_LEVEL, MAX_MIP_LEVELS);

            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            // be sure to set minification filter to mip_linear
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
            glGenerateTextureMipmap(m_cubeTexture);
        }

        {
            auto programId = ProgramRegistry::get().getProgram(SHADER_PREFILTER_CUBE_MAP);
            auto* program = Program::get(programId);

            program->prepareRT();
            program->bind();
            state.bindTexture(UNIT_ENVIRONMENT_MAP, m_envCubeMapID, false);

            render(program, m_cubeTexture, m_size);

            state.unbindTexture(UNIT_ENVIRONMENT_MAP, false);
            state.invalidateAll();
        }
    }

    void PrefilterMap::bindTexture(
        kigl::GLState& state,
        int unitIndex)
    {
        state.bindTexture(unitIndex, m_cubeTexture, false);
    }

    void PrefilterMap::render(
        Program* program,
        int cubeTextureID,
        int baseSize)
    {
        auto& state = kigl::GLState::get();

        const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        const glm::mat4 captureViews[] =
        {
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        const glm::vec4 clearColor{ 0.f };
        const float clearDepth{ 1.f };

        const TextureCube& cube = TextureCube::get();

        kigl::GLFrameBufferHandle captureFBO;
        kigl::GLRenderBufferHandle rbo;
        {
            captureFBO.create("capture_fbo");
            rbo.create("capture_rbo");

            glNamedFramebufferRenderbuffer(captureFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
            glNamedFramebufferDrawBuffer(captureFBO, GL_COLOR_ATTACHMENT0);
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, captureFBO);

        for (unsigned int mip = 0; mip < MAX_MIP_LEVELS; ++mip)
        {
            // reisze framebuffer according to mip-level size.
            unsigned int mipSize = static_cast<unsigned int>(baseSize * std::pow(0.5, mip));

            const float roughness = (float)mip / (float)(MAX_MIP_LEVELS - 1);
            program->setFloat("u_roughness", roughness);

            {
                glNamedRenderbufferStorage(rbo, GL_DEPTH_COMPONENT24, mipSize, mipSize);
                glViewport(0, 0, mipSize, mipSize);
            }

            for (unsigned int face = 0; face < 6; ++face)
            {
                auto projected = captureProjection * captureViews[face];
                program->setMat4("projected", projected);

                // NOTE KI side vs. face difference
                // https://stackoverflow.com/questions/55169053/opengl-render-to-cubemap-using-dsa-direct-state-access
                glNamedFramebufferTextureLayer(
                    captureFBO,
                    GL_COLOR_ATTACHMENT0,
                    cubeTextureID,
                    mip,
                    face);

                glClearNamedFramebufferfv(captureFBO, GL_COLOR, 0, glm::value_ptr(clearColor));
                glClearNamedFramebufferfv(captureFBO, GL_DEPTH, 0, &clearDepth);

                cube.draw();
            }
        }

        {
            auto* program = Program::get(ProgramRegistry::get().getProgram(SHADER_FLAT_CUBE_MAP));

            program->prepareRT();
            program->bind();
            state.bindTexture(UNIT_EDITOR_CUBE_MAP, m_cubeTexture, false);

            const glm::ivec2 flatSize{ m_size * 4 * 0.25f, m_size * 3 * 0.25f };

            std::unique_ptr<render::FrameBuffer> captureFBO{ nullptr };
            {
                auto buffer = new render::FrameBuffer(
                    "captureFBO",
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
            att.textureID = 0;
            att.createdTexture = false;

            state.unbindTexture(UNIT_EDITOR_CUBE_MAP, m_cubeTexture);
        }
    }
}
