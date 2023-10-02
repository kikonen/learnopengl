#include "NodeRenderer.h"
#include "NodeRenderer.h"

#include "asset/Program.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"

#include "component/Camera.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"

#include "kigl/GLStencilMode.h"


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

    //m_selectionProgramPointSprite = m_registry->m_programRegistry->getProgram(SHADER_SELECTION_POINT_SPRITE, { { DEF_USE_ALPHA, "1" } });
    //m_selectionProgramPointSprite->prepare(assets);
}

void NodeRenderer::updateView(const RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;

    // NOTE KI keep same scale as in gbuffer to allow glCopyImageSubData
    int w = ctx.m_assets.gBufferScale * res.x;
    int h = ctx.m_assets.gBufferScale * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = w != m_width || h != m_height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO(fmt::format("NODE_BUFFER: update - w={}, h={}", w, h));

    {
        auto buffer = new FrameBuffer(
            "node",
            {
                w, h,
                {
                    // NOTE KI alpha NOT needed
                    FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT0),
                    // NOTE KI depth/stencil needed only for highlight/selecction
                    FrameBufferAttachment::getDepthStencilRbo(),
                }
            });

        m_buffer.reset(buffer);
        m_buffer->prepare();
    }

    m_width = w;
    m_height = h;
}

void NodeRenderer::render(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    ctx.validateRender("node_map");

    m_taggedCount = ctx.m_assets.showTagged ? ctx.m_registry->m_nodeRegistry->countTagged() : 0;
    m_selectedCount = ctx.m_assets.showSelection ? ctx.m_registry->m_nodeRegistry->countSelected() : 0;

    {
        targetBuffer->clearAll();

        renderStencil(ctx, targetBuffer);
        {
            ctx.m_nodeDraw->drawNodes(
                ctx,
                targetBuffer,
                [](const MeshType* type) { return true; },
                [](const Node* node) { return true; },
                NodeDraw::KIND_ALL,
                // NOTE KI nothing to clear; keep stencil, depth copied from gbuffer
                GL_COLOR_BUFFER_BIT);
        }
        renderHighlight(ctx, targetBuffer);
    }
}

// Render selected nodes into stencil mask
void NodeRenderer::renderStencil(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    if (!ctx.m_assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    targetBuffer->bind(ctx);

    ctx.m_state.enableStencil(GLStencilMode::fill(STENCIL_HIGHLIGHT));

    // draw entity data mask
    {
        //m_selectionProgramPointSprite->bind(ctx.m_state);
        //m_selectionProgramPointSprite->u_stencilMode->set(STENCIL_MODE_MASK);

        m_selectionProgram->bind(ctx.m_state);
        m_selectionProgram->u_stencilMode->set(STENCIL_MODE_MASK);

        ctx.m_nodeDraw->drawProgram(
            ctx,
            m_selectionProgram,
            //m_selectionProgramPointSprite,
            nullptr,
            [](const MeshType* type) { return true; },
            [&ctx](const Node* node) { return node->isHighlighted(ctx.m_assets); },
            NodeDraw::KIND_ALL);
    }
    ctx.m_batch->flush(ctx);

    ctx.m_state.disableStencil();
}

// Render highlight over stencil masked nodes
void NodeRenderer::renderHighlight(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    if (!ctx.m_assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    targetBuffer->bind(ctx);

    ctx.m_state.setEnabled(GL_DEPTH_TEST, false);
    ctx.m_state.enableStencil(GLStencilMode::except(STENCIL_HIGHLIGHT));

    // draw selection color (scaled a bit bigger)
    {
        //m_selectionProgramPointSprite->bind(ctx.m_state);
        //m_selectionProgramPointSprite->u_stencilMode->set(STENCIL_MODE_HIGHLIGHT);

        m_selectionProgram->bind(ctx.m_state);
        m_selectionProgram->u_stencilMode->set(STENCIL_MODE_HIGHLIGHT);

        // draw all selected nodes with stencil
        ctx.m_nodeDraw->drawProgram(
            ctx,
            m_selectionProgram,
            //m_selectionProgramPointSprite,
            nullptr,
            [](const MeshType* type) { return true; },
            [&ctx](const Node* node) { return node->isHighlighted(ctx.m_assets); },
            NodeDraw::KIND_ALL);
    }
    ctx.m_batch->flush(ctx);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);

    ctx.m_state.setEnabled(GL_DEPTH_TEST, true);
    ctx.m_state.disableStencil();
}
