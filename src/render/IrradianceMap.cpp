#include "IrradianceMap.h"

#include "kigl/kigl.h"

#include "asset/Shader.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Program.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"

#include "CubeRender.h"


void IrradianceMap::prepareRT(
    const Assets& assets,
    Registry* registry)
{
    if (m_envCubeMapID <= 0) return;

    m_size = assets.irradianceMapSize;

    {
        m_cubeTexture.create("cube_map", GL_TEXTURE_CUBE_MAP, m_size, m_size);

        glTextureStorage2D(m_cubeTexture, 1, GL_RGB16F, m_size, m_size);

        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // be sure to set minification filter to mip_linear
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    {
        kigl::GLState state;

        auto program = registry->m_programRegistry->getProgram(SHADER_IRRADIANCE_CUBE_MAP);
        program->prepareRT(assets);

        program->bind(state);
        state.bindTexture(UNIT_ENVIRONMENT_MAP, m_envCubeMapID, false);

        CubeRender renderer;
        renderer.render(state, program, m_cubeTexture, m_size);
    }

    {
        // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
        glGenerateTextureMipmap(m_cubeTexture);
    }
}

void IrradianceMap::bindTexture(const RenderContext& ctx, int unitIndex)
{
    ctx.m_state.bindTexture(unitIndex, m_cubeTexture, false);
}
