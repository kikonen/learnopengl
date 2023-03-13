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
    m_selectionProgram->prepare(assets);

    m_selectionProgramSprite = m_registry->m_programRegistry->getProgram(SHADER_SELECTION_SPRITE, { { DEF_USE_ALPHA, "1" } });
    m_selectionProgramSprite->prepare(assets);
}

void NodeRenderer::render(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    m_taggedCount = ctx.assets.showTagged ? ctx.m_registry->m_nodeRegistry->countTagged() : 0;
    m_selectedCount = ctx.assets.showSelection ? ctx.m_registry->m_nodeRegistry->countSelected() : 0;

    {
        const glm::vec4 clearColor{ 0.0f, 0.0f, 1.0f, 0.0f };
        targetBuffer->bind(ctx);
        targetBuffer->clear(ctx, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, clearColor);

        renderStencil(ctx, targetBuffer);
        {
            ctx.m_nodeDraw->drawNodes(
                ctx,
                targetBuffer,
                false,
                [](const MeshType* type) { return true; },
                [&ctx](const Node* node) {
                    return true; // !node->isHighlighted(ctx.assets);
                },
                // NOTE KI nothing to clear; keep stencil, depth copied from gbuffer
                GL_DEPTH_BUFFER_BIT,
                clearColor);
        }
        {
            ctx.m_nodeDraw->drawBlended(
                ctx,
                targetBuffer,
                [](const MeshType* type) { return true; },
                [](const Node* node) { return true; });
        }
        renderHighlight(ctx, targetBuffer);
    }
}

// Render selected nodes into stencil mask
void NodeRenderer::renderStencil(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    if (!ctx.assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    targetBuffer->bind(ctx);

    ctx.state.enable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    // draw entity data mask
    {
        const glm::vec4 clearColor{ 0.0f, 1.0f, 1.0f, 0.0f };

        m_selectionProgramSprite->bind(ctx.state);
        u_stencilModeSprite.set(STENCIL_MODE_MASK);

        m_selectionProgram->bind(ctx.state);
        u_stencilMode.set(STENCIL_MODE_MASK);

        ctx.m_nodeDraw->drawProgram(
            ctx,
            m_selectionProgram,
            m_selectionProgramSprite,
            [](const MeshType* type) { return true; },
            [&ctx](const Node* node) { return node->isHighlighted(ctx.assets); });
    }
    ctx.m_batch->flush(ctx);

    ctx.state.disable(GL_STENCIL_TEST);

    glStencilMask(0x00);
}

// Render highlight over stencil masked nodes
void NodeRenderer::renderHighlight(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    if (!ctx.assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    targetBuffer->bind(ctx);

    ctx.state.enable(GL_STENCIL_TEST);
    ctx.state.disable(GL_DEPTH_TEST);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0x00);

    // draw selection color (scaled a bit bigger)
    {
        m_selectionProgramSprite->bind(ctx.state);
        u_stencilModeSprite.set(STENCIL_MODE_HIGHLIGHT);

        m_selectionProgram->bind(ctx.state);
        u_stencilMode.set(STENCIL_MODE_HIGHLIGHT);

        // draw all selected nodes with stencil
        ctx.m_nodeDraw->drawProgram(
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
