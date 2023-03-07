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

namespace
{
}

NodeRenderer::NodeRenderer()
{
}

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

    //ctx.state.enable(GL_CLIP_DISTANCE0);
    //ClipPlaneUBO& clip = ctx.clipPlanes.clipping[0];
    //clip.enabled = true;
    //clip.plane = glm::vec4(0, -1, 0, 15);
    //ctx.bindClipPlanesUBO();

    {
        // NOTE KI multitarget *WAS* just to support ObjectID, which is now separate renderer
        // => If program needs it need to define some logic
        int bufferCount = 1;

        GLenum buffers[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
        };
        if (bufferCount > 1) {
            glDrawBuffers(bufferCount, buffers);
            {
                // NOTE KI this was *ONLY* for ObjectID case
                //glm::vec4 bg{ 0.f, 0.f, 0.f, 1.f };
                //glClearBufferfv(GL_COLOR, 1, glm::value_ptr(bg));
            }
        }

        renderStencil(ctx);
        drawNodes(ctx, false);
        drawBlended(ctx);
        renderSelection(ctx);

        if (bufferCount > 1) {
            glDrawBuffers(1, buffers);
        }
    }

    //clip.enabled = false;
    //ctx.bindClipPlanesUBO();
    //ctx.state.disable(GL_CLIP_DISTANCE0);
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
    drawNodes(ctx, true);
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
    drawStencil(ctx);
    ctx.m_batch->flush(ctx);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);

    ctx.state.enable(GL_DEPTH_TEST);
    ctx.state.disable(GL_STENCIL_TEST);
}

// draw all non selected nodes
void NodeRenderer::drawNodes(
    const RenderContext& ctx,
    bool selection)
{
    NodeDraw draw;
    draw.drawNodes(
        ctx,
        selection,
        [](const MeshType* type) { return true; },
        [&ctx, selection](const Node* node) {
            const auto match = node->isHighlighted(ctx.assets);
            return selection ? match : !match;
        });
}

// draw all selected nodes with stencil
void NodeRenderer::drawStencil(const RenderContext& ctx)
{
    NodeDraw draw;
    draw.drawStencil(
        ctx,
        m_selectionProgram,
        m_selectionProgramSprite,
        [](const MeshType* type) { return true; },
        [&ctx](const Node* node) { return node->isHighlighted(ctx.assets); });
}

void NodeRenderer::drawBlended(
    const RenderContext& ctx)
{
    NodeDraw draw;
    draw.drawBlended(
        ctx,
        [](const MeshType* type) { return true; },
        [](const Node* node) { return true; });
}
