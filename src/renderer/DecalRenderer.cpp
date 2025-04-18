#include "DecalRenderer.h"

#include "asset/Assets.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "engine/PrepareContext.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"

#include "render/RenderContext.h"

#include "backend/gl/DrawElementsIndirectCommand.h"

#include "decal/DecalSystem.h"
#include "decal/DecalCollection.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"

namespace {
    inline const std::string SHADER_SOLID_DECAL{ "g_decal" };
    inline const std::string SHADER_BLEND_DECAL{ "blend_decal" };
}

DecalRenderer::DecalRenderer(bool useFrameStep)
    : Renderer("main", useFrameStep)
{
}

DecalRenderer::~DecalRenderer() = default;


void DecalRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    const auto& assets = Assets::get();
    m_enabled = assets.decalEnabled;

    Renderer::prepareRT(ctx);

    if (false) {
        m_blendDecalProgramId = ProgramRegistry::get().getProgram(
            SHADER_BLEND_DECAL,
            { { DEF_USE_DECAL, "1" },
              { DEF_USE_ALPHA, "1" },
              { DEF_USE_BLEND, "1" },
              { DEF_USE_NORMAL_TEX, "1" },
              { DEF_USE_TBN, "1" },
              { DEF_USE_PARALLAX, "1" },
            });
    }

    m_alphaDecalProgramId = ProgramRegistry::get().getProgram(
        SHADER_SOLID_DECAL,
        { { DEF_USE_DECAL, "1" },
          { DEF_USE_ALPHA, "1" },
          { DEF_USE_NORMAL_TEX, "1" },
          { DEF_USE_TBN, "1" },
          { DEF_USE_PARALLAX, "1" },
        });

    m_solidDecalProgramId = ProgramRegistry::get().getProgram(
        SHADER_SOLID_DECAL);
}

void DecalRenderer::renderSolid(
    const RenderContext& ctx)
{
    if (!isEnabled()) return;

    auto& collections = decal::DecalSystem::get().getCollections();
    for (auto& coll : collections) {
        renderSolidCollection(ctx, coll);
    }
}

void DecalRenderer::renderBlend(
    const RenderContext& ctx)
{
    if (!isEnabled()) return;

    auto& collections = decal::DecalSystem::get().getCollections();
    for (auto& coll : collections) {
        renderBlendCollection(ctx, coll);
    }
}

void DecalRenderer::renderSolidCollection(
    const RenderContext& ctx,
    const decal::DecalCollection& collection)
{
    if (!isEnabled()) return;

    const auto instanceCount = collection.getActiveDecalCount();
    if (instanceCount == 0) return;

    collection.bind();

    auto& state = ctx.m_state;
    const bool lineMode = ctx.m_forceLineMode;

    {
        // NOTE KI decals don't update depth
        // => For linemode draw visualization of cube
        state.setDepthMask(lineMode ? GL_TRUE : GL_FALSE);

        // NOET KI no decals over skybox
        // => draw *before* skybox, thus pointless
        if (lineMode) {
            state.setStencil(kigl::GLStencilMode::fill(STENCIL_SOLID | STENCIL_FOG));
        }
        else {
            state.setStencil({});
        }
    }

    state.polygonFrontAndBack(lineMode ? GL_LINE : GL_FILL);
    state.frontFace(GL_CCW);
    state.setEnabled(GL_BLEND, false);
    //state.setEnabled(GL_CULL_FACE, false);

    if (lineMode) {
        Program::get(m_solidDecalProgramId)->bind();
    }
    else {
        Program::get(m_alphaDecalProgramId)->bind();
    }

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 14, instanceCount);

    if (!lineMode) {
        state.setDepthMask(GL_TRUE);
    }
}

void DecalRenderer::renderBlendCollection(
    const RenderContext& ctx,
    const decal::DecalCollection& collection)
{
    if (!isEnabled()) return;

    const auto instanceCount = collection.getActiveDecalCount();
    if (instanceCount == 0) return;

    collection.bind();

    auto& state = ctx.m_state;
    const bool lineMode = ctx.m_forceLineMode;

    if (!lineMode) {
        // NOTE KI decals don't update depth
        state.setDepthMask(lineMode ? GL_TRUE : GL_FALSE);
        state.setEnabled(GL_BLEND, true);

        // https://stackoverflow.com/questions/31850635/opengl-additive-blending-get-issue-when-no-background
        //state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_DST_ALPHA, GL_ZERO, GL_DST_ALPHA });
        //state.setBlendMode({});
        state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });
    }

    state.polygonFrontAndBack(lineMode ? GL_LINE : GL_FILL);
    //state.setEnabled(GL_CULL_FACE, false);

    if (lineMode) {
        Program::get(m_solidDecalProgramId)->bind();
    }
    else {
        Program::get(m_blendDecalProgramId)->bind();
    }

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 14, instanceCount);

    if (!lineMode) {
        state.setDepthMask(GL_TRUE);
        state.setEnabled(GL_BLEND, false);
    }
}
