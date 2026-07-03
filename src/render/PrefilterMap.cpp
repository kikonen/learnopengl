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

#include "util/Log.h"


namespace {
    inline const std::string SHADER_PREFILTER_CUBE_MAP{ "prefilter_cube_map" };
    inline const std::string SHADER_FLAT_CUBE_MAP{ "flat_cube_map" };

    // DEBUG KI diagnose "texture (0) on unit 70 / no defined base level" warning.
    // Logs the actual GL state of the source cube bound to UNIT_ENVIRONMENT_MAP.
    void debugEnvCube(const char* where, int envCubeMapID, int outputCubeID, int mip)
    {
        GLboolean isTex = glIsTexture(static_cast<GLuint>(envCubeMapID));
        GLint w = -1, h = -1, baseLevel = -1, maxLevel = -1, minFilter = 0;
        if (isTex) {
            glGetTextureLevelParameteriv(envCubeMapID, 0, GL_TEXTURE_WIDTH, &w);
            glGetTextureLevelParameteriv(envCubeMapID, 0, GL_TEXTURE_HEIGHT, &h);
            glGetTextureParameteriv(envCubeMapID, GL_TEXTURE_BASE_LEVEL, &baseLevel);
            glGetTextureParameteriv(envCubeMapID, GL_TEXTURE_MAX_LEVEL, &maxLevel);
            glGetTextureParameteriv(envCubeMapID, GL_TEXTURE_MIN_FILTER, &minFilter);
        }

        // what is *actually* bound to texture image unit 70 right now
        GLint prevActive = 0;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActive);
        glActiveTexture(GL_TEXTURE0 + UNIT_ENVIRONMENT_MAP);
        GLint boundCube = -1;
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &boundCube);
        glActiveTexture(prevActive);

        KI_INFO(fmt::format(
            "ENVCUBE_DEBUG[{}]: envCubeMapID={}, isTexture={}, level0={}x{}, base={}, max={}, minFilter=0x{:x}, unit70_boundCube={}, outputCube={}, mip={}",
            where, envCubeMapID, (int)isTex, w, h, baseLevel, maxLevel, minFilter, boundCube, outputCubeID, mip));
    }
}

namespace render {
    //With 256 base size and 5 mip levels :
    //mip 0 : 256x256  roughness 0.00
    //    mip 1 : 128x128  roughness 0.25
    //    mip 2 : 64x64   roughness 0.50
    //    mip 3 : 32x32   roughness 0.75
    //    mip 4 : 16x16   roughness 1.00
    void PrefilterMap::prepareRT(
        const PrepareContext& ctx)
    {
        const auto& assets = ctx.getAssets();

        if (m_envCubeMapID <= 0) return;

        createRT(assets.prefilterMapSize);
        convolve(m_envCubeMapID);
    }

    void PrefilterMap::createRT(int size)
    {
        if (m_cubeTexture.valid()) return;

        m_size = size;

        m_cubeTexture.create("prefilter_map", GL_TEXTURE_CUBE_MAP, m_size, m_size);

        // https://www.khronos.org/opengl/wiki/Common_Mistakes#Creating_a_complete_texture
        glTextureStorage2D(m_cubeTexture, MAX_MIP_LEVELS, GL_RGB16F, m_size, m_size);

        // https://stackoverflow.com/questions/37232110/opengl-cubemap-writing-to-mipmap
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_BASE_LEVEL, 0);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAX_LEVEL, MAX_MIP_LEVELS - 1);

        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // be sure to set minification filter to mip_linear
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void PrefilterMap::convolve(int envCubeMapID)
    {
        if (envCubeMapID <= 0 || !m_cubeTexture.valid()) return;

        auto& state = kigl::GLState::get();

        auto programId = ProgramRegistry::get().getProgram(SHADER_PREFILTER_CUBE_MAP);
        auto* program = Program::get(programId);

        program->prepareRT();
        program->bind();
        // NOTE KI force: GLState caches per-unit texture ids and reused GL names
        // (after a texture delete) can make a non-forced bind a no-op, leaving unit
        // UNIT_ENVIRONMENT_MAP at texture 0 => "sampler on undefined texture" warning.
        state.bindTexture(UNIT_ENVIRONMENT_MAP, envCubeMapID, true);

        if (m_debug) debugEnvCube("prefilter.convolve", envCubeMapID, m_cubeTexture, -1);

        render(program, m_cubeTexture, m_size);

        state.unbindTexture(UNIT_ENVIRONMENT_MAP, false);
        // NOTE KI reset the current program: convolve leaves this program bound while unit
        // UNIT_ENVIRONMENT_MAP is now unbound, so a later draw (main pass) would validate a
        // samplerCube against texture 0 => GL undefined-behavior warning. invalidateAll()
        // only clears the cache; glUseProgram(0) actually detaches it.
        state.useProgram(0);
        state.invalidateAll();
    }

    void PrefilterMap::convolveMip(int envCubeMapID, int mip)
    {
        if (envCubeMapID <= 0 || !m_cubeTexture.valid()) return;
        if (mip < 0 || mip >= MAX_MIP_LEVELS) return;

        auto& state = kigl::GLState::get();

        auto programId = ProgramRegistry::get().getProgram(SHADER_PREFILTER_CUBE_MAP);
        auto* program = Program::get(programId);

        program->prepareRT();
        program->bind();
        // NOTE KI force: GLState caches per-unit texture ids and reused GL names
        // (after a texture delete) can make a non-forced bind a no-op, leaving unit
        // UNIT_ENVIRONMENT_MAP at texture 0 => "sampler on undefined texture" warning.
        state.bindTexture(UNIT_ENVIRONMENT_MAP, envCubeMapID, true);

        if (m_debug) debugEnvCube("prefilter.convolveMip", envCubeMapID, m_cubeTexture, mip);

        renderMip(program, m_cubeTexture, m_size, mip);

        state.unbindTexture(UNIT_ENVIRONMENT_MAP, false);
        // NOTE KI reset the current program: convolve leaves this program bound while unit
        // UNIT_ENVIRONMENT_MAP is now unbound, so a later draw (main pass) would validate a
        // samplerCube against texture 0 => GL undefined-behavior warning. invalidateAll()
        // only clears the cache; glUseProgram(0) actually detaches it.
        state.useProgram(0);
        state.invalidateAll();
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
        for (int mip = 0; mip < MAX_MIP_LEVELS; ++mip) {
            renderMip(program, cubeTextureID, baseSize, mip);
        }
    }

    void PrefilterMap::renderMip(
        Program* program,
        int cubeTextureID,
        int baseSize,
        int mip)
    {
        auto& state = kigl::GLState::get();

        // NOTE KI cube drawn from inside-out
        state.frontFace(GL_CW);

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

        // NOTE KI create + wire the capture FBO/RBO once; both create() calls are
        // idempotent and the attachment only needs to be set on first creation. The depth
        // RBO is allocated once at the largest (mip 0) size and reused for every smaller mip
        // (attachments may differ in size in GL 3.0+; the viewport clamps the render area).
        // Reallocating it per mip triggered a driver storage-alloc every frame.
        const bool firstUse = m_captureFBO.m_fbo <= 0;
        m_captureFBO.create("capture_fbo");
        m_captureRBO.create("capture_rbo");
        if (firstUse) {
            glNamedRenderbufferStorage(m_captureRBO, GL_DEPTH_COMPONENT24, baseSize, baseSize);
            glNamedFramebufferRenderbuffer(m_captureFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);
            glNamedFramebufferDrawBuffer(m_captureFBO, GL_COLOR_ATTACHMENT0);
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_captureFBO);

        // render area for this mip level; depth RBO (mip 0 size) covers it
        const unsigned int mipSize = static_cast<unsigned int>(baseSize * std::pow(0.5, mip));

        const float roughness = (float)mip / (float)(MAX_MIP_LEVELS - 1);
        program->setFloat("u_roughness", roughness);

        glViewport(0, 0, mipSize, mipSize);

        for (unsigned int face = 0; face < 6; ++face)
        {
            auto projected = captureProjection * captureViews[face];
            program->setMat4("u_projected", projected);

            // NOTE KI side vs. face difference
            // https://stackoverflow.com/questions/55169053/opengl-render-to-cubemap-using-dsa-direct-state-access
            glNamedFramebufferTextureLayer(
                m_captureFBO,
                GL_COLOR_ATTACHMENT0,
                cubeTextureID,
                mip,
                face);

            glClearNamedFramebufferfv(m_captureFBO, GL_COLOR, 0, glm::value_ptr(clearColor));
            glClearNamedFramebufferfv(m_captureFBO, GL_DEPTH, 0, &clearDepth);

            cube.draw();
        }

        // NOTE KI cube drawn from inside-out
        state.frontFace(GL_CCW);
    }
}
