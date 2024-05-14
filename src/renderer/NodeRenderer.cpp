#include "NodeRenderer.h"
#include "NodeRenderer.h"

#include "asset/Assets.h"
#include "asset/Program.h"
#include "asset/ProgramUniforms.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "kigl/GLState.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "component/Camera.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ProgramRegistry.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"

#include "kigl/GLStencilMode.h"


void NodeRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    const auto& assets = ctx.m_assets;

    m_renderFrameStart = assets.nodeRenderFrameStart;
    m_renderFrameStep = assets.nodeRenderFrameStep;

    m_selectionProgram = ProgramRegistry::get().getProgram(SHADER_SELECTION, { { DEF_USE_ALPHA, "1" } });
    m_selectionProgram->prepareRT();

    //m_selectionProgramPointSprite = ProgramRegistry::get().getProgram(SHADER_SELECTION_POINT_SPRITE, { { DEF_USE_ALPHA, "1" } });
    //m_selectionProgramPointSprite->prepare(assets);
}

void NodeRenderer::updateRT(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;

    const auto& res = ctx.m_resolution;

    // NOTE KI keep same scale as in gbuffer to allow glCopyImageSubData
    int w = (int)(assets.gBufferScale * res.x);
    int h = (int)(assets.gBufferScale * res.y);
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = w != m_width || h != m_height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO(fmt::format("NODE_BUFFER: update - w={}, h={}", w, h));

    {
        auto buffer = new render::FrameBuffer(
            m_name + "_node",
            {
                w, h,
                {
                    // NOTE KI alpha NOT needed
                    render::FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT0),
                    // NOTE KI depth/stencil needed only for highlight/selecction
                    render::FrameBufferAttachment::getDepthStencilRbo(),
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
    render::FrameBuffer* targetBuffer)
{
    const auto& assets = ctx.m_assets;
    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;

    ctx.validateRender("node_map");

    m_taggedCount = assets.showTagged ? nodeRegistry.countTagged() : 0;
    m_selectedCount = assets.showSelection ? nodeRegistry.countSelected() : 0;

    {
        targetBuffer->clearAll();

        fillHighlightMask(ctx, targetBuffer);
        {
            ctx.m_nodeDraw->drawNodes(
                ctx,
                targetBuffer,
                [](const mesh::MeshType* type) { return true; },
                [](const Node* node) { return true; },
                render::NodeDraw::KIND_ALL,
                // NOTE KI nothing to clear; keep stencil, depth copied from gbuffer
                GL_COLOR_BUFFER_BIT);
        }
        renderHighlight(ctx, targetBuffer);
    }
}

// Render selected nodes into stencil mask
void NodeRenderer::fillHighlightMask(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& assets = ctx.m_assets;

    if (!assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    auto& state = ctx.m_state;

    targetBuffer->bind(ctx);

    state.setStencil(kigl::GLStencilMode::fill(STENCIL_HIGHLIGHT));

    // draw entity data mask
    {
        //m_selectionProgramPointSprite->bind();
        //m_selectionProgramPointSprite->u_stencilMode->set(STENCIL_MODE_MASK);

        auto* program = m_selectionProgram;
        program->bind();
        program->m_uniforms->u_stencilMode.set(STENCIL_MODE_MASK);

        ctx.m_nodeDraw->drawProgram(
            ctx,
            [this, &program](const mesh::MeshType* type) {
                auto* p = type->m_selectionProgram ? type->m_selectionProgram : program;
                if (p != program) {
                    p->bind();
                    p->m_uniforms->u_stencilMode.set(STENCIL_MODE_MASK);
                }
                return p;
            },
            [](const mesh::MeshType* type) { return true; },
            [&ctx](const Node* node) { return node->isHighlighted(); },
            render::NodeDraw::KIND_ALL);
    }
    ctx.m_batch->flush(ctx);
}

// Render highlight over stencil masked nodes
void NodeRenderer::renderHighlight(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& assets = ctx.m_assets;

    if (!assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    auto& state = ctx.m_state;

    targetBuffer->bind(ctx);

    state.setEnabled(GL_DEPTH_TEST, false);
    state.setStencil(kigl::GLStencilMode::except(STENCIL_HIGHLIGHT));

    // draw selection color (scaled a bit bigger)
    {
        //m_selectionProgramPointSprite->bind();
        //m_selectionProgramPointSprite->u_stencilMode->set(STENCIL_MODE_HIGHLIGHT);

        auto* program = m_selectionProgram;
        program->bind();
        program->m_uniforms->u_stencilMode.set(STENCIL_MODE_HIGHLIGHT);

        // draw all selected nodes with stencil
        ctx.m_nodeDraw->drawProgram(
            ctx,
            [this, &program](const mesh::MeshType* type) {
                auto* p = type->m_selectionProgram ? type->m_selectionProgram : program;
                if (p != program) {
                    p->bind();
                    p->m_uniforms->u_stencilMode.set(STENCIL_MODE_HIGHLIGHT);
                }
                return p;
            },
            [](const mesh::MeshType* type) { return true; },
            [&ctx](const Node* node) { return node->isHighlighted(); },
            render::NodeDraw::KIND_ALL);
    }
    ctx.m_batch->flush(ctx);

    state.setEnabled(GL_DEPTH_TEST, true);
}
