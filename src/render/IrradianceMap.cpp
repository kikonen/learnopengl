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
#include "render/FrameBuffer.h"
#include "render/ScreenTri.h"

#include "registry/Registry.h"

#include "CubeRender.h"

namespace {
    inline const std::string SHADER_IRRADIANCE_CUBE_MAP{ "irradiance_cube_map" };
    inline const std::string SHADER_FLAT_CUBE_MAP{ "flat_cube_map" };
}

namespace render {
    void IrradianceMap::prepareRT(
        const PrepareContext& ctx)
    {
        const auto& assets = ctx.getAssets();

        if (m_envCubeMapID <= 0) return;

        createRT(assets.irradianceMapSize);
        convolve(m_envCubeMapID);
    }

    void IrradianceMap::createRT(int size)
    {
        if (m_cubeTexture.valid()) return;

        m_size = size;

        m_cubeTexture.create("irradiance_map", GL_TEXTURE_CUBE_MAP, m_size, m_size);

        glTextureStorage2D(m_cubeTexture, 1, GL_RGB16F, m_size, m_size);

        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // NOTE KI LINEAR *REQUIRED*
        // => interpolation is needed to avoid square pattern
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void IrradianceMap::convolve(int envCubeMapID)
    {
        if (envCubeMapID <= 0 || !m_cubeTexture.valid()) return;

        auto& state = kigl::GLState::get();

        auto* program = Program::get(ProgramRegistry::get().getProgram(SHADER_IRRADIANCE_CUBE_MAP));
        program->prepareRT();

        program->bind();
        state.bindTexture(UNIT_ENVIRONMENT_MAP, envCubeMapID, false);

        CubeRender renderer;
        renderer.render(program, m_cubeTexture, m_size);

        state.unbindTexture(UNIT_ENVIRONMENT_MAP, false);
        state.invalidateAll();
    }

    void IrradianceMap::bindTexture(
        kigl::GLState& state,
        int unitIndex)
    {
        state.bindTexture(unitIndex, m_cubeTexture, false);
    }
}
