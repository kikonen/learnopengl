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
#include "render/ScreenTri.h"

#include "shader/DataUBO.h"

#include "kigl/GLStencilMode.h"

namespace {
    // NOTE KI selection mask resolution vs. layer buffer. OUTLINE_WIDTH in
    // screen_selection_outline_pass.fs is in *mask* texels, thus changing this
    // scales the outline thickness with it
    constexpr float SELECTION_MASK_SCALE = 0.5f;

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

    if (m_useHighlight) {
        auto outlineProgramId = ProgramRegistry::get().getProgram(SHADER_SELECTION_OUTLINE_PASS);
        m_selectionOutlineProgram = Program::get(outlineProgramId);
        m_selectionOutlineProgram->prepareRT();
    }
}

void LayerRenderer::updateView(const UpdateViewContext& ctx)
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

        m_nodeDraw->updateView(ctx, bufferScale);
    }


    {
        if (m_useHighlight) {
            m_frameBuffer = util::Ref<render::FrameBuffer>::create(
                fmt::format("{}_layer_{}x{}", m_name, w, h),
                render::FrameBufferSpecification {
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
            m_frameBuffer = util::Ref<render::FrameBuffer>::create(
                fmt::format("{}_layer_{}x{}", m_name, w, h),
                render::FrameBufferSpecification {
                    w, h,
                    {
                    // NOTE KI alpha NEEDED for layers
                    render::FrameBufferAttachment::getEffectTextureHdr(GL_COLOR_ATTACHMENT0),
                }
                });
        }

        m_frameBuffer->prepare();

        // NOTE KI ensure buffer is cleared initially
        {
            auto& state = kigl::GLState::get();
            state.setStencil({});

            m_frameBuffer->invalidateAll();
            m_frameBuffer->clearAll();
        }
    }

    if (m_useHighlight) {
        // NOTE KI smaller buffer to save memory; outline is only a few pixels
        // wide, and LINEAR sampling of the smaller mask gives a *smoother*
        // silhouette edge than a 1:1 mask would
        int mw = (int)(w * SELECTION_MASK_SCALE);
        int mh = (int)(h * SELECTION_MASK_SCALE);
        if (mw < 1) mw = 1;
        if (mh < 1) mh = 1;

        // NOTE KI *NO* depth/stencil; silhouette does not need them
        m_maskBuffer = util::Ref<render::FrameBuffer>::create(
            fmt::format("{}_selection_mask_{}x{}", m_name, mw, mh),
            render::FrameBufferSpecification {
                mw, mh,
                {
                render::FrameBufferAttachment::getSelectionMaskTexture(GL_COLOR_ATTACHMENT0),
            }
            });

        m_maskBuffer->prepare();
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

        targetBuffer->invalidateAll();
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

        targetBuffer->invalidateAll();
        targetBuffer->clearAll();

        {
            // NOTE KI when using wireframe selection, exclude selected objects from normal
            // draw. Common case (no wireframe / nothing selected) keeps the trivial acceptor
            // to avoid building a set + per-drawable lookup every frame.
            std::function<bool(const render::DrawableInfo&)> drawableSelector = render::ACCEPT_ALL_DRAWABLES;
            bool filtersDrawables = false;
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
                    filtersDrawables = true;
                }
            }

            render::DrawContext drawContext{
                drawableSelector,
                render::KIND_ALL,
                // NOTE KI nothing to clear; keep stencil, depth copied from gbuffer
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                // when set, the selector is folded into the cull instead of called per pass
                filtersDrawables
            };

            m_nodeDraw->drawNodes(
                ctx,
                drawContext,
                targetBuffer);
        }

        // NOTE KI *MUST* be after drawNodes; both selection passes rely on the
        // visibility bits computed by cullFrustum inside drawNodes for *this*
        // layer camera. Before it the bits are leftovers from whichever pass
        // culled last (shadow/water/mirror/cubemap run on own frame steps, each
        // with own camera) => selected node randomly missing from the mask.
        if (m_useHighlight) {
            if (useWireframeSelection) {
                renderSelectionWireframe(ctx, targetBuffer);
            } else {
                fillHighlightMask(ctx, targetBuffer);
                renderHighlight(ctx, targetBuffer);
            }
        }
    }
}

// Render selected nodes into selection mask buffer
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

    // NOTE KI mask has own buffer; bind *before* clear (DSA clear of an unbound
    // FBO is not reliable on all drivers)
    m_maskBuffer->bind(localCtx);
    m_maskBuffer->clearAll();

    // NOTE KI silhouette only; no depth/stencil involved at all
    state.setStencil({});
    state.setEnabled(GL_DEPTH_TEST, false);
    state.setDepthMask(GL_FALSE);

    // NOTE KI mask alpha must be the raw coverage; runs after drawNodes, thus
    // blend state cannot be assumed to be off
    state.setEnabled(GL_BLEND, false);
    state.setBlendMode({});

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
            // own selector over the shared cull; default require-mask ignores VISIBLE_SELECTED
            &drawContext.drawableSelector,
            drawContext.kindBits,
            render::ROUTE_ALL);
    }
    localCtx.m_batch->flush(localCtx);

    state.setDepthMask(GL_TRUE);
    state.setEnabled(GL_DEPTH_TEST, true);
}

// Render outline around silhouettes in selection mask
//
// https://www.reddit.com/r/opengl/comments/14jisvu/how_can_i_outline_selected_meshes/
// https://ameye.dev/notes/rendering-outlines/
// NOTE KI screen space dilate of the mask, replaces the old "shift mode"
// approach (4x geometry redraw + stencil), which needed the layer stencil to
// survive the depth copy from gbuffer
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

    targetBuffer->bind(localCtx);

    // NOTE KI pure screen space pass; outline pixels are picked by the shader
    // via discard, thus no depth/stencil needed
    state.setEnabled(GL_DEPTH_TEST, false);
    state.setDepthMask(GL_FALSE);
    state.setStencil({});
    state.frontFace(GL_CCW);
    state.polygonFrontAndBack(GL_FILL);

    // NOTE KI outline edge is *anti-aliased*, i.e. partial coverage; blend it
    // over existing layer content instead of overwriting. Alpha uses
    // ONE/ONE_MINUS_SRC_ALPHA so that coverage accumulates correctly for the
    // later layer composite (layer buffer alpha is meaningful)
    state.setEnabled(GL_BLEND, true);
    state.setBlendMode({
        GL_FUNC_ADD,
        GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
        GL_ONE, GL_ONE_MINUS_SRC_ALPHA });

    m_maskBuffer->bindTexture(state, ATT_MASK_INDEX, UNIT_SELECTION_MASK);

    m_selectionOutlineProgram->bind();
    render::ScreenTri::get().draw();

    state.setBlendMode({});
    state.setEnabled(GL_BLEND, false);
    state.setDepthMask(GL_TRUE);
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
        // wireframe draws exactly the SELECTED nodes (which the main cull excluded from
        // VISIBLE_SELECTED); default require-mask VISIBLE_ALL ignores that bit, so its own
        // selector still finds them.
        &drawableSelector,
        render::KIND_ALL,
        render::ROUTE_ALL);

    localCtx.m_batch->flush(localCtx);
}
