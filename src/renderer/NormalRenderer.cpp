#include "NormalRenderer.h"

#include "asset/Shader.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "NodeDraw.h"


void NormalRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);

    m_normalProgram = m_registry->m_programRegistry->getProgram(SHADER_NORMAL);
    m_normalProgram->prepare(assets);
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
            m_normalProgram,
            m_normalProgram,
            [](const MeshType* type) { return !type->m_flags.tessellation && type->m_entityType != EntityType::sprite; },
            [](const Node* node) { return true; });
    }

    ctx.m_batch->flush(ctx);
}
