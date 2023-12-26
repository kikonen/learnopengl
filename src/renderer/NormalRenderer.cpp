#include "NormalRenderer.h"

#include "asset/Shader.h"
#include "asset/Program.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ProgramRegistry.h"



void NormalRenderer::prepareView(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareView(assets, registry);

    m_normalProgram = m_registry->m_programRegistry->getProgram(SHADER_NORMAL);
    m_normalProgram->prepareView(assets);
}

void NormalRenderer::render(
    const RenderContext& ctx)
{
    drawNodes(ctx);
}

void NormalRenderer::drawNodes(const RenderContext& ctx)
{
    {
        ctx.m_nodeDraw->drawProgram(
            ctx,
            [this](const MeshType* type) { return m_normalProgram; },
            [](const MeshType* type) {
                return type->m_flags.noNormals &&
                    !type->m_flags.tessellation &&
                    type->m_entityType != EntityType::point_sprite;
            },
            [](const Node* node) { return true; },
            NodeDraw::KIND_ALL);
    }

    ctx.m_batch->flush(ctx);
}
