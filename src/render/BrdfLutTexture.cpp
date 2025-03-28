#include "BrdfLutTexture.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "engine/PrepareContext.h"

#include "render/TextureQuad.h"
#include "render/FrameBuffer.h"
#include "render/RenderContext.h"

#include "registry/Registry.h"

namespace {
    inline const std::string SHADER_BRDF_LUT{ "brdf_lut" };
}

namespace render {
    void BrdfLutTexture::prepareRT(
        const PrepareContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;
        auto& state = kigl::GLState::get();

        m_size = assets.brdfLutSize;

        {
            m_texture.create("brdf_lut", GL_TEXTURE_2D, m_size, m_size);

            glTextureStorage2D(m_texture, 1, GL_RG16F, m_size, m_size);

            glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        {
            auto* program = Program::get(ProgramRegistry::get().getProgram(SHADER_BRDF_LUT));

            program->prepareRT();
            program->bind();
            render(program, m_texture, m_size);

            state.invalidateAll();
        }
    }

    void BrdfLutTexture::bindTexture(
        kigl::GLState& state,
        int unitIndex)
    {
        state.bindTexture(unitIndex, m_texture, false);
    }

    void BrdfLutTexture::render(
        Program* program,
        int textureID,
        int size)
    {
        const glm::vec4 clearColor{ 0.f };
        const float clearDepth{ 1.f };

        std::unique_ptr<FrameBuffer> captureFBO{ nullptr };
        {
            auto buffer = new FrameBuffer(
                "captureFBO",
                {
                    size, size,
                    {
                        FrameBufferAttachment::getDrawBuffer(),
                        FrameBufferAttachment::getDepthRbo(),
                    }
                });
            captureFBO.reset(buffer);
            captureFBO->prepare();

            glNamedFramebufferTexture(*captureFBO, GL_COLOR_ATTACHMENT0, textureID, 0);
        }

        {
            glViewport(0, 0, size, size);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *captureFBO);

            const glm::vec4 clearColor{ 0.f };
            const float clearDepth{ 1.f };

            glClearNamedFramebufferfv(*captureFBO, GL_COLOR, 0, glm::value_ptr(clearColor));
            glClearNamedFramebufferfv(*captureFBO, GL_DEPTH, 0, &clearDepth);

            TextureQuad::get().draw();
        }
    }
}
