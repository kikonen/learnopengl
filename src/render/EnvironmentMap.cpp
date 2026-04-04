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

    constexpr unsigned int MAX_MIP_LEVELS = 5;
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

            glTextureStorage2D(m_cubeTexture, MAX_MIP_LEVELS, GL_RGB16F, m_size, m_size);

            // https://stackoverflow.com/questions/37232110/opengl-cubemap-writing-to-mipmap
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_BASE_LEVEL, 0);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAX_LEVEL, MAX_MIP_LEVELS - 1);

            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            // NOTE KI LINEAR *REQUIRED*
            // => mipmap interpolation is needed to avoid square pattern
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

        {
            glGenerateTextureMipmap(m_cubeTexture);
        }
    }

    void EnvironmentMap::bindTexture(
        kigl::GLState& state,
        int unitIndex)
    {
        state.bindTexture(unitIndex, m_cubeTexture, false);
    }
}
