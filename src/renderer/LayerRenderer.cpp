#include "LayerRenderer.h"

#include "asset/Assets.h"

#include "shader/Program.h"
#include "shader/ProgramUniforms.h"
#include "shader/Shader.h"
#include "shader/Uniform.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "render/DebugContext.h"
#include "render/Camera.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"
#include "render/CollectionRender.h"
#include "render/DrawContext.h"

#include "kigl/GLStencilMode.h"

LayerRenderer::~LayerRenderer() = default;

void LayerRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    const auto& assets = ctx.m_assets;

    {
        m_nodeDraw = std::make_unique<render::NodeDraw>();

        auto& pipeline = m_nodeDraw->m_pipeline;
        //pipeline.m_particle = false;
        //pipeline.m_decal = false;
        //pipeline.m_fog = false;
        //pipeline.m_emission = false;
        //pipeline.m_bloom = false;

        m_nodeDraw->prepareRT(ctx);
    }

    m_renderFrameStart = assets.nodeRenderFrameStart;
    m_renderFrameStep = assets.nodeRenderFrameStep;

    auto selectionProgramId = ProgramRegistry::get().getProgram(SHADER_SELECTION, { { DEF_USE_ALPHA, "1" } });
    m_selectionProgram = Program::get(selectionProgramId);
    m_selectionProgram->prepareRT();
}

void LayerRenderer::updateRT(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;
    auto& dbg = render::DebugContext::get();

    int w;
    int h;
    {
        const auto& res = ctx.m_resolution;
        const auto bufferScale = dbg.getGBufferScale();

        // NOTE KI keep same scale as in gbuffer to allow glCopyImageSubData
        w = (int)(bufferScale * res.x);
        h = (int)(bufferScale * res.y);
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        bool changed = w != m_width || h != m_height;
        if (!changed) return;

        m_width = w;
        m_height = h;

        KI_INFO(fmt::format("NODE_BUFFER: update - w={}, h={}", w, h));

        m_nodeDraw->updateRT(ctx, bufferScale);
    }


    {
        render::FrameBuffer* buffer{ nullptr };

        if (m_useHighlight) {
            buffer = new render::FrameBuffer(
                m_name + "_node",
                {
                    w, h,
                    {
                    // NOTE KI alpha NEEDED for layers
                    render::FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT0),
                    // NOTE KI depth/stencil needed only for highlight/selecction
                    render::FrameBufferAttachment::getDepthStencilRbo(),
                }
                });
        }
        else {
            buffer = new render::FrameBuffer(
                m_name + "_node",
                {
                    w, h,
                    {
                    // NOTE KI alpha NEEDED for layers
                    render::FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT0),
                }
                });
        }

        m_buffer.reset(buffer);
        m_buffer->prepare();

        // NOTE KI ensure buffer is cleared initially
        {
            auto& state = kigl::GLState::get();
            state.setStencil({});

            m_buffer->clearAll();
        }
    }
}

void LayerRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    auto& state = ctx.m_state;

    if (!isEnabled())
    {
        state.setStencil({});
        targetBuffer->clearAll();
        return;
    }

    const auto& assets = ctx.m_assets;
    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;
    auto& selectionRegistry = *ctx.m_registry->m_selectionRegistry;

    ctx.validateRender("node_map");

    if (m_useHighlight) {
        m_taggedCount = assets.showTagged ? selectionRegistry.getTaggedCount() : 0;
        m_selectedCount = assets.showSelection ? selectionRegistry.getSelectedCount() : 0;
    }

    {
        targetBuffer->clearAll();

        if (m_useHighlight) {
            fillHighlightMask(ctx, targetBuffer);
        }

        {
            render::DrawContext drawContext{
                [](const mesh::MeshType* type) { return true; },
                [](const Node* node) { return true; },
                render::KIND_ALL,
                // NOTE KI nothing to clear; keep stencil, depth copied from gbuffer
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
            };

            m_nodeDraw->drawNodes(
                ctx,
                drawContext,
                targetBuffer);
        }

        if (m_useHighlight) {
            renderHighlight(ctx, targetBuffer);
        }
    }
}

// Render selected nodes into stencil mask
void LayerRenderer::fillHighlightMask(
    const RenderContext& parentCtx,
    render::FrameBuffer* targetBuffer)
{
    RenderContext ctx{ "local", &parentCtx };
    ctx.m_forceSolid = true;

    const auto& assets = ctx.m_assets;

    if (!assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    auto& selectionRegistry = *ctx.m_registry->m_selectionRegistry;

    auto& state = ctx.m_state;

    targetBuffer->bind(ctx);

    state.setStencil(kigl::GLStencilMode::fill(STENCIL_HIGHLIGHT));

    // draw entity data mask
    {
        auto* program = m_selectionProgram;

        render::DrawContext drawContext{
            [](const mesh::MeshType* type) { return true; },
            [&selectionRegistry](const Node* node) {
                return selectionRegistry.isHighlighted(node->toHandle());
            },
            render::KIND_ALL,
            0
        };

        render::CollectionRender collectionRender;
        collectionRender.drawProgram(
            ctx,
            [this, &program](const mesh::LodMesh& lodMesh) {
                auto* p = lodMesh.m_selectionProgramId ? Program::get(lodMesh.m_selectionProgramId) : program;
                p->m_uniforms->u_stencilMode.set(STENCIL_MODE_SHIFT_NONE);
                return p->m_id;
            },
            drawContext.typeSelector,
            drawContext.nodeSelector,
            drawContext.kindBits);
    }
    ctx.m_batch->flush(ctx);
}

// Render highlight over stencil masked nodes
void LayerRenderer::renderHighlight(
    const RenderContext& parentCtx,
    render::FrameBuffer* targetBuffer)
{
    RenderContext ctx{ "local", &parentCtx };
    ctx.m_forceSolid = true;

    const auto& assets = ctx.m_assets;

    if (!assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    auto& state = ctx.m_state;

    auto& selectionRegistry = *ctx.m_registry->m_selectionRegistry;

    targetBuffer->bind(ctx);

    state.setEnabled(GL_DEPTH_TEST, false);
    state.setStencil(kigl::GLStencilMode::except(STENCIL_HIGHLIGHT));

    const int SHIFTS[] = {
        //STENCIL_MODE_SHIFT_NONE,
        STENCIL_MODE_SHIFT_UP,
        STENCIL_MODE_SHIFT_LEFT,
        STENCIL_MODE_SHIFT_RIGHT,
        STENCIL_MODE_SHIFT_DOWN,
    };

    // draw selection color (scaled a bit bigger)
    // https://www.reddit.com/r/opengl/comments/14jisvu/how_can_i_outline_selected_meshes/
    // https://ameye.dev/notes/rendering-outlines/
    // NOTE KI using "shift mode" approach, based into "hell engine"
    for (const auto shift : SHIFTS) {
        auto* program = m_selectionProgram;

        render::DrawContext drawContext{
            [](const mesh::MeshType* type) { return true; },
            [&selectionRegistry](const Node* node) {
                return selectionRegistry.isHighlighted(node->toHandle());
            },
            render::KIND_ALL
        };

        // draw all selected nodes with stencil
        render::CollectionRender collectionRender;
        collectionRender.drawProgram(
            ctx,
            [this, &program, shift](const mesh::LodMesh& lodMesh) {
                auto* p = lodMesh.m_selectionProgramId ? Program::get(lodMesh.m_selectionProgramId) : program;
                p->m_uniforms->u_stencilMode.set(shift);
                return p->m_id;
            },
            drawContext.typeSelector,
            drawContext.nodeSelector,
            drawContext.kindBits);
        ctx.m_batch->flush(ctx);
    }

    state.setEnabled(GL_DEPTH_TEST, true);
}
