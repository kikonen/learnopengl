#include "NormalRenderer.h"

#include "shader/Program.h"
#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "engine/PrepareContext.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/CollectionRender.h"
#include "render/DrawContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
}

void NormalRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    // NOTE KI bones are pare case, but for conveniency it's global here
    m_normalProgramId = ProgramRegistry::get().getProgram(SHADER_NORMAL, { {DEF_USE_BONES, "1"} });
}

void NormalRenderer::render(
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    if (!isEnabled()) return;
    targetBuffer->bind(ctx);
    drawNodes(ctx);
}

void NormalRenderer::drawNodes(const render::RenderContext& ctx)
{
    ctx.m_batch->flush(ctx);

    {
        // NOTE KI stencil mask must be cleared
        // BUG KI normals are showing only if selection/volume is shown some node
        ctx.m_state.setStencil({});

        render::DrawContext drawContext{
            [](const model::Node* node) { return !node->m_typeFlags.noNormals; },
            render::KIND_ALL
        };

        render::CollectionRender collectionRender;
        collectionRender.drawProgram(
            ctx,
            [this](const mesh::LodMesh& lodMesh) {
                if (lodMesh.m_flags.tessellation) return (ki::program_id)0;
                return lodMesh.m_normalProgramId ? lodMesh.m_normalProgramId : m_normalProgramId;
            },
            [](ki::program_id programId) {},
            drawContext.nodeSelector,
            drawContext.kindBits);
    }

    ctx.m_batch->flush(ctx);
}
