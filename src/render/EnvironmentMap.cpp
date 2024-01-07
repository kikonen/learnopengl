#include "EnvironmentMap.h"

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Program.h"

#include "engine/PrepareContext.h"
#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"

#include "CubeRender.h"

namespace render {
    void EnvironmentMap::prepareRT(
        const PrepareContext& ctx,
        int size)
    {
        auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;

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
            kigl::GLState state;

            auto program = registry->m_programRegistry->getProgram(SHADER_HDRI_CUBE_MAP);
            program->prepareRT(assets);

            program->bind(state);
            state.bindTexture(UNIT_HDR_TEXTURE, m_hdriTextureID, false);

            CubeRender renderer;
            renderer.render(state, program, m_cubeTexture, m_size);

            state.unbindTexture(UNIT_HDR_TEXTURE, false);
        }
    }

    void EnvironmentMap::bindTexture(const RenderContext& ctx, int unitIndex)
    {
        ctx.m_state.bindTexture(unitIndex, m_cubeTexture, false);
    }
}
