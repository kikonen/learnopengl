#include "NormalRenderer.h"

#include "asset/Shader.h"
#include "asset/Program.h"

#include "kigl/GLState.h"

#include "engine/PrepareContext.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ProgramRegistry.h"



void NormalRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    m_normalProgram = ProgramRegistry::get().getProgram(SHADER_NORMAL, { {DEF_USE_BONES, "1"} });
    m_normalProgram->prepareRT();
}

void NormalRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    if (!isEnabled()) return;
    targetBuffer->bind(ctx);
    drawNodes(ctx);
}

void NormalRenderer::drawNodes(const RenderContext& ctx)
{
    ctx.m_batch->flush(ctx);

    {
        // NOTE KI stencil mask must be cleared
        // BUG KI normals are showing only if selection/volume is shown some node
        ctx.m_state.setStencil({});

        ctx.m_nodeDraw->drawProgram(
            ctx,
            [this](const mesh::LodMesh& lodMesh) {
                if (lodMesh.m_flags.tessellation) return (Program*)nullptr;
                return m_normalProgram;
            },
            [](const mesh::MeshType* type) {
                return !type->m_flags.noNormals;
            },
            [](const Node* node) { return true; },
            render::KIND_ALL);
    }

    ctx.m_batch->flush(ctx);
}
