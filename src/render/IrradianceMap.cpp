#include "IrradianceMap.h"

#include "kigl/kigl.h"


#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Assets.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "engine/PrepareContext.h"
#include "render/RenderContext.h"

#include "registry/Registry.h"

#include "CubeRender.h"


namespace render {
    void IrradianceMap::prepareRT(
        const PrepareContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;
        auto& state = kigl::GLState::get();

        if (m_envCubeMapID <= 0) return;

        m_size = assets.irradianceMapSize;

        {
            m_cubeTexture.create("irradiance_map", GL_TEXTURE_CUBE_MAP, m_size, m_size);

            glTextureStorage2D(m_cubeTexture, 1, GL_RGB16F, m_size, m_size);

            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            // be sure to set minification filter to mip_linear
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        {
            auto* program = ProgramRegistry::get().getProgram(SHADER_IRRADIANCE_CUBE_MAP);
            program->prepareRT();

            program->bind();
            state.bindTexture(UNIT_ENVIRONMENT_MAP, m_envCubeMapID, false);

            CubeRender renderer;
            renderer.render(program, m_cubeTexture, m_size);

            state.unbindTexture(UNIT_ENVIRONMENT_MAP, false);
            state.clear();
        }

        {
            // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
            glGenerateTextureMipmap(m_cubeTexture);
        }
    }

    void IrradianceMap::bindTexture(const RenderContext& ctx, int unitIndex)
    {
        auto& state = kigl::GLState::get();
        state.bindTexture(unitIndex, m_cubeTexture, false);
    }
}
