#include "NormalRenderer.h"

#include "shader/Program.h"
#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

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


void NormalRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    m_normalProgramId = ProgramRegistry::get().getProgram(SHADER_NORMAL, { {DEF_USE_BONES, "1"} });
    Program::get(m_normalProgramId)->prepareRT();
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
                if (lodMesh.m_flags.tessellation) return (ki::program_id)0;
                return m_normalProgramId;
            },
            [](const mesh::MeshType* type) {
                return !type->m_flags.noNormals;
            },
            [](const Node* node) { return true; },
            render::KIND_ALL);
    }

    ctx.m_batch->flush(ctx);
}
