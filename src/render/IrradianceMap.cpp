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

#include "util/Log.h"

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
        // NOTE KI force: GLState caches per-unit texture ids and reused GL names
        // (after a texture delete) can make a non-forced bind a no-op, leaving unit
        // UNIT_ENVIRONMENT_MAP at texture 0 => "sampler on undefined texture" warning.
        state.bindTexture(UNIT_ENVIRONMENT_MAP, envCubeMapID, true);

        if (m_debug) {
            // DEBUG KI diagnose unit-70 "texture (0) / no base level" warning
            GLboolean isTex = glIsTexture(static_cast<GLuint>(envCubeMapID));
            GLint w = -1, h = -1;
            if (isTex) {
                glGetTextureLevelParameteriv(envCubeMapID, 0, GL_TEXTURE_WIDTH, &w);
                glGetTextureLevelParameteriv(envCubeMapID, 0, GL_TEXTURE_HEIGHT, &h);
            }
            GLint prevActive = 0;
            glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActive);
            glActiveTexture(GL_TEXTURE0 + UNIT_ENVIRONMENT_MAP);
            GLint boundCube = -1;
            glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &boundCube);
            glActiveTexture(prevActive);
            KI_INFO(fmt::format(
                "ENVCUBE_DEBUG[irradiance.convolve]: envCubeMapID={}, isTexture={}, level0={}x{}, unit70_boundCube={}, outputCube={}",
                envCubeMapID, (int)isTex, w, h, boundCube, (int)m_cubeTexture));
        }

        m_cubeRender.render(program, m_cubeTexture, m_size);

        state.unbindTexture(UNIT_ENVIRONMENT_MAP, false);
        // NOTE KI reset the current program: convolve leaves this program bound while unit
        // UNIT_ENVIRONMENT_MAP is now unbound, so a later clear/draw (e.g. PassDeferred::
        // initRender -> clearAll) validates a samplerCube against texture 0 => GL
        // undefined-behavior warning. invalidateAll() only clears the cache.
        state.useProgram(0);
        state.invalidateAll();
    }

    void IrradianceMap::bindTexture(
        kigl::GLState& state,
        int unitIndex)
    {
        state.bindTexture(unitIndex, m_cubeTexture, false);
    }
}
