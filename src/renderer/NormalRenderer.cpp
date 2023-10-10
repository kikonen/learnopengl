#include "NormalRenderer.h"

#include "asset/Shader.h"
#include "asset/Program.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"



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
            [this](const MeshType* type) { return m_normalProgram; },
            [](const MeshType* type) { return !type->m_flags.tessellation && type->m_entityType != EntityType::point_sprite; },
            [&ctx](const Node* node) {
                return node->m_id != ctx.m_assets.volumeUUID &&
                    node->m_id != ctx.m_assets.cubeMapUUID &&
                    node->m_id != ctx.m_assets.skyboxUUID;
            },
            NodeDraw::KIND_ALL);
    }

    ctx.m_batch->flush(ctx);
}
