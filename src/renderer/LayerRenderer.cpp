#include "LayerRenderer.h"

#include <unordered_set>

#include "asset/Assets.h"

#include "shader/Program.h"
#include "shader/ProgramUniforms.h"
#include "shader/Shader.h"
#include "shader/Uniform.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "mesh/LodMesh.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "debug/DebugContext.h"

#include "render/Camera.h"
#include "render/RenderContext.h"
#include "render/RenderData.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/DrawableInfo.h"
#include "render/NodeDraw.h"
#include "render/CollectionRender.h"
#include "render/DrawContext.h"

#include "shader/DataUBO.h"

#include "kigl/GLStencilMode.h"

namespace {
    // selection/tag handles -> entityIndex set (entityIndex == NodeHandle::m_handleIndex)
    std::unordered_set<uint32_t> toEntitySet(const std::vector<pool::NodeHandle>& handles)
    {
        std::unordered_set<uint32_t> set;
        set.reserve(handles.size());
        for (const auto& h : handles) set.insert(h.m_handleIndex);
        return set;
    }
}

LayerRenderer::~LayerRenderer() = default;

void LayerRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    const auto& assets = ctx.getAssets();

    {
        m_nodeDraw = std::make_unique<render::NodeDraw>(fmt::format("{}_layer_draw", m_name));

        auto& pipeline = m_nodeDraw->m_pipeline;
        //pipeline.m_particle = false;
        //pipeline.m_decal = false;
        //pipeline.m_fog = false;
        //pipeline.m_emission = false;
        //pipeline.m_bloom = false;
        pipeline.m_debug = true;
        pipeline.m_debugPhysics = true;
        pipeline.m_debugVolume = true;
        pipeline.m_debugEnvironmentProbe = true;
        pipeline.m_debugNormal = true;
        pipeline.m_debugSocket = true;

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
    const auto& assets = ctx.getAssets();
    auto& dbg = debug::DebugContext::get();

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

        KI_INFO(fmt::format("LAYER_BUFFER: update - w={}, h={}", w, h));

        m_nodeDraw->updateRT(ctx, bufferScale);
    }


    {
        render::FrameBuffer* buffer{ nullptr };

        if (m_useHighlight) {
            buffer = new render::FrameBuffer(
                fmt::format("{}_layer_{}x{}", m_name, w, h),
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
                fmt::format("{}_layer_{}x{}", m_name, w, h),
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
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    auto& state = ctx.getGLState();

    if (!isEnabled())
    {
        state.setStencil({});
        targetBuffer->clearAll();
        return;
    }

    const auto& assets = ctx.getAssets();
    auto& nodeRegistry = *ctx.getRegistry()->m_nodeRegistry;
    auto& selectionRegistry = *ctx.getRegistry()->m_selectionRegistry;

    ctx.validateRender("layer");

    if (m_useHighlight) {
        m_taggedCount = assets.showTagged ? selectionRegistry.getTaggedCount() : 0;
        m_selectedCount = assets.showSelection ? selectionRegistry.getSelectedCount() : 0;
    }

    auto& dbg = debug::DebugContext::get();
    const bool useWireframeSelection = m_useHighlight && dbg.m_selectionWireframe;

    {
        state.setStencil({});
        targetBuffer->clearAll();

        // NOTE KI skip stencil mask when using wireframe selection
        if (m_useHighlight && !useWireframeSelection) {
            fillHighlightMask(ctx, targetBuffer);
        }

        {
            // NOTE KI when using wireframe selection, exclude selected objects from normal
            // draw. Common case (no wireframe / nothing selected) keeps the trivial acceptor
            // to avoid building a set + per-drawable lookup every frame.
            std::function<bool(const render::DrawableInfo&)> drawableSelector = render::ACCEPT_ALL_DRAWABLES;
            if (useWireframeSelection) {
                std::unordered_set<uint32_t> excluded;
                if (assets.showTagged)
                    for (const auto& h : selectionRegistry.getTagged()) excluded.insert(h.m_handleIndex);
                if (assets.showSelection)
                    for (const auto& h : selectionRegistry.getSelected()) excluded.insert(h.m_handleIndex);
                if (!excluded.empty()) {
                    drawableSelector = [excluded = std::move(excluded)](const render::DrawableInfo& d) {
                        return excluded.find(d.entityIndex) == excluded.end();
                    };
                }
            }

            render::DrawContext drawContext{
                drawableSelector,
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
            if (useWireframeSelection) {
                renderSelectionWireframe(ctx, targetBuffer);
            } else {
                renderHighlight(ctx, targetBuffer);
            }
        }
    }
}

// Render selected nodes into stencil mask
void LayerRenderer::fillHighlightMask(
    const render::RenderContext& parentCtx,
    render::FrameBuffer* targetBuffer)
{
    render::RenderContext localCtx{ "local", &parentCtx };
    localCtx.m_forceSolid = true;

    const auto& assets = localCtx.getAssets();

    if (!assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    auto& selectionRegistry = *localCtx.getRegistry()->m_selectionRegistry;

    auto& state = localCtx.getGLState();

    targetBuffer->bind(localCtx);

    state.setStencil(kigl::GLStencilMode::fill(STENCIL_HIGHLIGHT));

    // draw entity data mask
    {
        const bool showTagged = assets.showTagged;
        const bool showSelection = assets.showSelection;
        const std::function<bool(const render::DrawableInfo&)> drawableSelector =
            [showTagged, showSelection,
             tagged = toEntitySet(selectionRegistry.getTagged()),
             selected = toEntitySet(selectionRegistry.getSelected())](const render::DrawableInfo& d) {
                bool accept = showTagged || showSelection;
                if (showTagged)
                    accept = accept && (tagged.find(d.entityIndex) != tagged.end());
                if (showSelection)
                    accept = accept && (selected.find(d.entityIndex) != selected.end());
                return accept;
            };

        render::DrawContext drawContext{
            drawableSelector,
            render::KIND_ALL,
            0
        };

        render::CollectionRender collectionRender;
        collectionRender.drawProgramWithPrepare(
            localCtx,
            [this](const render::DrawableInfo& drawable) {
                return drawable.selectionProgramId ? drawable.selectionProgramId : m_selectionProgram->m_id;
            },
            [this](ki::program_id programId) {
                auto* program = Program::get(programId);
                program->m_uniforms->u_stencilMode.set(STENCIL_MODE_SHIFT_NONE);
                program->m_uniforms->u_wireframeMode.set(false);
            },
            drawContext.drawableSelector,
            drawContext.kindBits);
    }
    localCtx.m_batch->flush(localCtx);
}

// Render highlight over stencil masked nodes
void LayerRenderer::renderHighlight(
    const render::RenderContext& parentCtx,
    render::FrameBuffer* targetBuffer)
{
    render::RenderContext localCtx{ "local", &parentCtx };
    localCtx.m_forceSolid = true;

    const auto& assets = localCtx.getAssets();

    if (!assets.showHighlight) return;
    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    auto& state = localCtx.getGLState();

    auto& selectionRegistry = *localCtx.getRegistry()->m_selectionRegistry;

    targetBuffer->bind(localCtx);

    state.setEnabled(GL_DEPTH_TEST, false);
    state.setStencil(kigl::GLStencilMode::except(STENCIL_HIGHLIGHT));

    const int SHIFTS[] = {
        //STENCIL_MODE_SHIFT_NONE,
        STENCIL_MODE_SHIFT_UP,
        STENCIL_MODE_SHIFT_LEFT,
        STENCIL_MODE_SHIFT_RIGHT,
        STENCIL_MODE_SHIFT_DOWN,
    };

    // built once; reused across the shift passes
    const bool showTagged = assets.showTagged;
    const bool showSelection = assets.showSelection;
    const std::function<bool(const render::DrawableInfo&)> drawableSelector =
        [showTagged, showSelection,
         tagged = toEntitySet(selectionRegistry.getTagged()),
         selected = toEntitySet(selectionRegistry.getSelected())](const render::DrawableInfo& d) {
            bool accept = showTagged || showSelection;
            if (showTagged)
                accept = accept && (tagged.find(d.entityIndex) != tagged.end());
            if (showSelection)
                accept = accept && (selected.find(d.entityIndex) != selected.end());
            return accept;
        };

    // draw selection color (scaled a bit bigger)
    // https://www.reddit.com/r/opengl/comments/14jisvu/how_can_i_outline_selected_meshes/
    // https://ameye.dev/notes/rendering-outlines/
    // NOTE KI using "shift mode" approach, based into "hell engine"
    for (const auto shift : SHIFTS) {
        render::DrawContext drawContext{
            drawableSelector,
            render::KIND_ALL
        };

        // draw all selected nodes with stencil
        render::CollectionRender collectionRender;
        collectionRender.drawProgramWithPrepare(
            localCtx,
            [this, shift](const render::DrawableInfo& drawable) {
                return drawable.selectionProgramId ? drawable.selectionProgramId : m_selectionProgram->m_id;
            },
            [this, shift](ki::program_id programId) {
                auto* program = Program::get(programId);
                program->m_uniforms->u_stencilMode.set(shift);
                program->m_uniforms->u_wireframeMode.set(false);
            },
            drawContext.drawableSelector,
            drawContext.kindBits);
        localCtx.m_batch->flush(localCtx);
    }

    state.setEnabled(GL_DEPTH_TEST, true);
}

// Render selected nodes in wireframe mode
void LayerRenderer::renderSelectionWireframe(
    const render::RenderContext& parentCtx,
    render::FrameBuffer* targetBuffer)
{
    render::RenderContext localCtx{ "wireframe_selection", &parentCtx };

    const auto& assets = localCtx.getAssets();

    if (m_taggedCount == 0 && m_selectedCount == 0) return;

    auto& selectionRegistry = *localCtx.getRegistry()->m_selectionRegistry;

    // NOTE KI force line mode for selected objects
    localCtx.m_forceLineMode = true;
    localCtx.m_allowLineMode = true;

    targetBuffer->bind(localCtx);

    const bool showTagged = assets.showTagged;
    const bool showSelection = assets.showSelection;
    const std::function<bool(const render::DrawableInfo&)> drawableSelector =
        [showTagged, showSelection,
         tagged = toEntitySet(selectionRegistry.getTagged()),
         selected = toEntitySet(selectionRegistry.getSelected())](const render::DrawableInfo& d) {
            if (showSelection && selected.find(d.entityIndex) != selected.end())
                return true;
            if (showTagged && tagged.find(d.entityIndex) != tagged.end())
                return true;
            return false;
        };

    render::CollectionRender collectionRender;
    collectionRender.drawProgramWithPrepare(
        localCtx,
        [this](const render::DrawableInfo& drawable) {
            return drawable.selectionProgramId ? drawable.selectionProgramId : m_selectionProgram->m_id;
        },
        [](ki::program_id programId) {
            auto* program = Program::get(programId);
            program->m_uniforms->u_stencilMode.set(STENCIL_MODE_SHIFT_NONE);
            program->m_uniforms->u_wireframeMode.set(true);
        },
        drawableSelector,
        render::KIND_ALL);

    localCtx.m_batch->flush(localCtx);
}
