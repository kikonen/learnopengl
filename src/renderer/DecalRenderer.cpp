#include "DecalRenderer.h"

#include "asset/Assets.h"
#include "asset/Program.h"
#include "asset/Shader.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "engine/PrepareContext.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"

#include "render/RenderContext.h"

#include "backend/gl/DrawElementsIndirectCommand.h"

#include "Decal/DecalSystem.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ProgramRegistry.h"
#include "registry/ModelRegistry.h"

namespace {
}

void DecalRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    const auto& assets = ctx.m_assets;
    m_enabled = assets.decalEnabled;

    Renderer::prepareRT(ctx);

    m_decalProgram = ProgramRegistry::get().getProgram(
        SHADER_DECAL,
        { { DEF_USE_ALPHA, "1" },
          { DEF_USE_BLEND, "1" } });

    m_decalProgram->prepareRT();

    {
        auto generator = mesh::PrimitiveGenerator::quad();
        m_quad = generator.create();
        m_quad->prepareVAO();
    }
}

void DecalRenderer::render(
    const RenderContext& ctx)
{
    if (!isEnabled()) return;

    auto& state = ctx.m_state;

    const auto instanceCount = decal::DecalSystem::get().getActiveDecalCount();
    if (instanceCount == 0) return;

    //KI_INFO_OUT(fmt::format("DECAL: count={}", instanceCount));


    if (false) {
    // NOTE KI decals don't update depth
    state.setDepthMask(GL_FALSE);
        state.setEnabled(GL_BLEND, true);
        //state.setEnabled(GL_CULL_FACE, false);

        //state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });

        // https://stackoverflow.com/questions/31850635/opengl-additive-blending-get-issue-when-no-background
        //state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_DST_ALPHA, GL_SRC_ALPHA, GL_DST_ALPHA });
        state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_DST_ALPHA, GL_ZERO, GL_DST_ALPHA });
        //glBlendFunc(GL_ONE, GL_ONE);
    }

    state.setEnabled(GL_CULL_FACE, false);
    const bool wireframe = ctx.m_forceWireframe;
    state.polygonFrontAndBack(wireframe ? GL_LINE : GL_FILL);

    m_decalProgram->bind();

    state.bindVAO(*m_quad->getVAO());

    glDrawElementsInstancedBaseVertexBaseInstance(
        GL_TRIANGLES,
        m_quad->getIndexCount(),
        GL_UNSIGNED_INT,
        (void*)(m_quad->getBaseIndex() * sizeof(GLuint)),
        instanceCount,
        m_quad->getBaseVertex(),
        m_quad->getBaseIndex());

    state.setEnabled(GL_CULL_FACE, true);


    if (false) {
    state.setDepthMask(GL_TRUE);
        state.setEnabled(GL_BLEND, false);
    }
}
