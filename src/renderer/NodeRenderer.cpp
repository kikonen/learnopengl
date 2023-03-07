#include "NodeRenderer.h"
#include "NodeRenderer.h"

#include "asset/Program.h"
#include "asset/Shader.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"

#include "component/Camera.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"

#include "NodeDraw.h"


void NodeRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);

    m_renderFrameStart = assets.nodeRenderFrameStart;
    m_renderFrameStep = assets.nodeRenderFrameStep;

    m_selectionProgram = m_registry->m_programRegistry->getProgram(SHADER_SELECTION, { { DEF_USE_ALPHA, "1" } });
    //m_selectionProgram->m_selection = true;
    m_selectionProgram->prepare(assets);

    m_selectionProgramSprite = m_registry->m_programRegistry->getProgram(SHADER_SELECTION_SPRITE, { { DEF_USE_ALPHA, "1" } });
    //m_selectionProgramSprite->m_selection = true;
    m_selectionProgramSprite->prepare(assets);
}

void NodeRenderer::render(
    const RenderContext& ctx)
{
    m_taggedCount = ctx.assets.showTagged ? ctx.m_registry->m_nodeRegistry->countTagged() : 0;
    m_selectedCount = ctx.assets.showSelection ? ctx.m_registry->m_nodeRegistry->countSelected() : 0;

    {
        renderStencil(ctx);
        {
            NodeDraw draw;
            draw.drawNodes(
                ctx,
                false,
                [](const MeshType* type) { return true; },
                [&ctx](const Node* node) {
                    return !node->isHighlighted(ctx.assets);
                });
        }
        {
            NodeDraw draw;
            draw.drawBlended(
                ctx,
                [](const MeshType* type) { return true; },
                [](const Node* node) { return true; });
        }
        renderSelection(ctx);
    }
}

void NodeRenderer::renderStencil(const RenderContext& ctx)
{
    if (!ctx.assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    ctx.m_batch->flush(ctx);

    ctx.state.enable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    // draw entity data mask
    {
        NodeDraw draw;
        draw.drawNodes(
            ctx,
            true,
            [](const MeshType* type) { return true; },
            [&ctx](const Node* node) {
                return node->isHighlighted(ctx.assets);
            });
    }

    ctx.m_batch->flush(ctx);

    ctx.state.disable(GL_STENCIL_TEST);

    glStencilMask(0x00);
}

void NodeRenderer::renderSelection(const RenderContext& ctx)
{
    if (!ctx.assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    ctx.m_batch->flush(ctx);

    ctx.state.enable(GL_STENCIL_TEST);
    ctx.state.disable(GL_DEPTH_TEST);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0x00);

    // draw selection color (scaled a bit bigger)
    {
        // draw all selected nodes with stencil
        NodeDraw draw;
        draw.drawProgram(
            ctx,
            m_selectionProgram,
            m_selectionProgramSprite,
            [](const MeshType* type) { return true; },
            [&ctx](const Node* node) { return node->isHighlighted(ctx.assets); });
    }
    ctx.m_batch->flush(ctx);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);

    ctx.state.enable(GL_DEPTH_TEST);
    ctx.state.disable(GL_STENCIL_TEST);
}
