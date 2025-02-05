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

#include "render/TextureQuad.h"
#include "render/RenderContext.h"

#include "backend/gl/DrawElementsIndirectCommand.h"

#include "Decal/DecalSystem.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"

namespace {
}

void DecalRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    const auto& assets = Assets::get();
    m_enabled = assets.decalEnabled;

    Renderer::prepareRT(ctx);

    m_blendDecalProgramId = ProgramRegistry::get().getProgram(
        SHADER_BLEND_DECAL,
        { { DEF_USE_DECAL, "1" },
          { DEF_USE_ALPHA, "1" },
          { DEF_USE_BLEND, "1" },
          { DEF_USE_NORMAL_TEX, "1" },
          { DEF_USE_TBN, "1" },
          { DEF_USE_PARALLAX, "1" },
        });

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

    auto& state = ctx.m_state;

    const auto instanceCount = decal::DecalSystem::get().getActiveDecalCount();
    if (instanceCount == 0) return;

    const bool lineMode = ctx.m_forceLineMode;
    state.polygonFrontAndBack(lineMode ? GL_LINE : GL_FILL);

    Program::get(m_alphaDecalProgramId)->bind();

    render::TextureQuad::get().drawInstanced(instanceCount);
}

void DecalRenderer::renderBlend(
    const RenderContext& ctx)
{
    if (!isEnabled()) return;

    const bool lineMode = ctx.m_forceLineMode;

    auto& state = ctx.m_state;

    const auto instanceCount = decal::DecalSystem::get().getActiveDecalCount();
    if (instanceCount == 0) return;

    if (!lineMode) {
        // NOTE KI decals don't update depth
        state.setDepthMask(GL_FALSE);
        state.setEnabled(GL_BLEND, true);

        // https://stackoverflow.com/questions/31850635/opengl-additive-blending-get-issue-when-no-background
        //state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_DST_ALPHA, GL_ZERO, GL_DST_ALPHA });
        state.setBlendMode({});
    }

    state.polygonFrontAndBack(lineMode ? GL_LINE : GL_FILL);
    //state.setEnabled(GL_CULL_FACE, false);

    if (lineMode) {
        Program::get(m_solidDecalProgramId)->bind();
    }
    else {
        Program::get(m_blendDecalProgramId)->bind();
    }

    render::TextureQuad::get().drawInstanced(instanceCount);

    if (!lineMode) {
        state.setDepthMask(GL_TRUE);
        state.setEnabled(GL_BLEND, false);
    }
}
