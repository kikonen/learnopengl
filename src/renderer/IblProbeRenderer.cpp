#include "IblProbeRenderer.h"

#include <limits>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <fmt/format.h>

#include "asset/Assets.h"
#include "asset/DynamicCubeMap.h"

#include "kigl/GLState.h"

#include "util/Log.h"

#include "shader/Shader.h"

#include "pool/NodeHandle.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/Snapshot.h"

#include "debug/DebugContext.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/NodeCollection.h"
#include "render/DrawContext.h"
#include "render/DrawableInfo.h"
#include "render/NodeDraw.h"
#include "render/CubeMap.h"
#include "render/CubeMapBuffer.h"

namespace {
    // standard cube-map capture orientation (matches the prefilter/irradiance convolution views)
    // +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back)
    const glm::vec3 CAMERA_FRONT[6] = {
        {  1,  0,  0 },
        { -1,  0,  0 },
        {  0,  1,  0 },
        {  0, -1,  0 },
        {  0,  0,  1 },
        {  0,  0, -1 },
    };

    const glm::vec3 CAMERA_UP[6] = {
        {  0, -1,  0 },
        {  0, -1,  0 },
        {  0,  0,  1 },
        {  0,  0, -1 },
        {  0, -1,  0 },
        {  0, -1,  0 },
    };

    // input radiance cube mip levels (matches PrefilterMap's MAX_MIP_LEVELS expectation)
    constexpr int CAPTURE_MIP_LEVELS = 5;
}

IblProbeRenderer::~IblProbeRenderer() = default;

void IblProbeRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    const auto& assets = ctx.getAssets();
    const auto& dbg = ctx.getDebug();

    {
        m_nodeDraw = std::make_unique<render::NodeDraw>(m_name);

        auto& pipeline = m_nodeDraw->m_pipeline;
        pipeline.m_particle = false;
        pipeline.m_decal = false;
        pipeline.m_fog = false;
        pipeline.m_emission = false;
        pipeline.m_bloom = false;
        pipeline.m_oit = false;
        pipeline.m_ssao = false;
        pipeline.m_effect = false;

        m_nodeDraw->prepareRT(ctx);
    }

    m_renderFrameStart = assets.environmentProbeRenderFrameStart;
    m_renderFrameStep = assets.environmentProbeRenderFrameStep;

    m_nearPlane = assets.cubeMapNearPlane;
    m_farPlane = assets.cubeMapFarPlane;

    m_envSize = assets.environmentMapSize;

    m_captureCube = std::make_unique<DynamicCubeMap>(
        fmt::format("{}_capture", m_name), m_envSize, CAPTURE_MIP_LEVELS);
    m_captureCube->prepareRT(ctx, false, { 0, 0, 0, 1.f });

    glm::vec3 origo{ 0 };
    for (int face = 0; face < 6; face++) {
        auto& camera = m_cameras.emplace_back(origo, CAMERA_FRONT[face], CAMERA_UP[face]);
        camera.setFov(90.f);
    }

    // allocate the IBL maps now; they are (re)convolved each bake
    m_irradianceMap.createRT(assets.irradianceMapSize);
    m_prefilterMap.createRT(assets.prefilterMapSize);

    setEnabled(dbg.m_environmentProbeEnabled);
}

void IblProbeRenderer::updateView(const UpdateViewContext& parentCtx)
{
    const auto& dbg = parentCtx.getDebug();
    setEnabled(dbg.m_environmentProbeEnabled);

    if (!isEnabled()) return;

    UpdateViewContext localCtx{
        parentCtx.getEngine(),
        m_envSize,
        m_envSize };

    m_nodeDraw->updateView(localCtx, 1.0f);
}

void IblProbeRenderer::bindTexture(kigl::GLState& state)
{
    // leave the skybox-derived maps (bound earlier this frame) as the fallback until first bake
    if (!isEnabled() || !m_baked) return;

    m_irradianceMap.bindTexture(state, UNIT_IRRADIANCE_MAP);
    m_prefilterMap.bindTexture(state, UNIT_PREFILTER_MAP);
}

bool IblProbeRenderer::render(
    const render::RenderContext& parentCtx)
{
    if (!isEnabled()) return false;
    if (!needRender(parentCtx)) return false;

    const int step = m_buildStep;

    // Start of a cycle: pick the probe origin once and hold it for all 6 faces, so a
    // single bake captures from one position (re-picked next cycle). Mixing across
    // cycles when the camera moves self-corrects within a cycle.
    if (step == 0) {
        enumerateProbes(parentCtx);

        model::Node* centerNode = findClosest(parentCtx);
        if (!centerNode) return false; // no probe yet -> stay at step 0

        const auto* snapshot = centerNode->getSnapshotRT();
        if (!snapshot) return false;

        m_centerNode = centerNode->toHandle();
        m_captureCenter = snapshot->getWorldPosition();
    }

    // DEBUG KI diagnose unit-70 warning: log capture cube identity + build step
    if (m_debug) {
        const int capId = static_cast<int>(m_captureCube->getTextureHandle());
        GLboolean isTex = glIsTexture(static_cast<GLuint>(capId));
        GLint w = -1, h = -1;
        if (isTex) {
            glGetTextureLevelParameteriv(capId, 0, GL_TEXTURE_WIDTH, &w);
            glGetTextureLevelParameteriv(capId, 0, GL_TEXTURE_HEIGHT, &h);
        }
        KI_INFO(fmt::format(
            "IBL_PROBE_STEP: step={}, captureCube={}, isTexture={}, level0={}x{}, envSize={}",
            step, capId, (int)isTex, w, h, m_envSize));
    }

    if (step < CAPTURE_STEPS) {
        // capture one face into the radiance cube. The capture's deferred pass reads the
        // IBL maps currently bound (last bake / skybox fallback) => single bounce.
        captureFace(parentCtx, step);
    }
    else if (step == IRRADIANCE_STEP) {
        // radiance cube fully captured this cycle -> mips (for prefilter sampling) + irradiance
        glGenerateTextureMipmap(m_captureCube->getTextureHandle());

        const int envCubeId = static_cast<int>(m_captureCube->getTextureHandle());
        m_irradianceMap.convolve(envCubeId);
    }
    else {
        const int mip = step - PREFILTER_STEP_BASE;
        const int envCubeId = static_cast<int>(m_captureCube->getTextureHandle());
        m_prefilterMap.convolveMip(envCubeId, mip);

        // first full cycle complete -> outputs are valid, start overriding skybox IBL
        if (mip == render::PrefilterMap::MAX_MIP_LEVELS - 1) {
            m_baked = true;
        }
    }

    m_buildStep = (step + 1) % BUILD_STEPS;
    m_rendered = true;
    return true;
}

void IblProbeRenderer::captureFace(
    const render::RenderContext& parentCtx,
    int face)
{
    auto* centerNode = m_centerNode.toNode();
    if (!centerNode) return;

    auto& camera = m_cameras[face];
    camera.setWorldPosition(m_captureCenter);

    render::RenderContext localCtx("IBL_PROBE",
        &parentCtx,
        &camera,
        m_nearPlane,
        m_farPlane,
        m_captureCube->m_size, m_captureCube->m_size);

    localCtx.m_useSsao = false;
    localCtx.m_useParticles = false;
    localCtx.m_useDecals = false;
    localCtx.m_useFog = false;
    localCtx.m_useEmission = false;
    localCtx.m_useBloom = false;
    localCtx.m_forceLineMode = false;

    auto targetBuffer = m_captureCube->asFrameBuffer(face);
    drawNodes(localCtx, &targetBuffer, centerNode);
}

void IblProbeRenderer::drawNodes(
    const render::RenderContext& ctx,
    render::CubeMapBuffer* targetBuffer,
    const model::Node* current)
{
    ctx.updateUBOs();
    ctx.bindDefaults();

    // TODO KI match special logic in CubeMapBuffer
    targetBuffer->bindFace();
    targetBuffer->clearAll();

    const auto currentEntityIndex = current->getEntityIndex();
    const auto currentId = current->getId();

    render::DrawContext drawContext{
        // opt-in: only `environment`-tagged static geometry (large surroundings); dynamic
        // characters and small/expensive clutter stay out. Skybox is a separate pass, so the
        // sky still fills the capture. Also skip the probe node itself / anything ignoring it.
        [currentEntityIndex, currentId](const render::DrawableInfo& d) {
            return d.m_flags.environment &&
                d.entityIndex != currentEntityIndex &&
                d.m_ignoredBy != currentId;
        },
        render::KIND_ALL,
        GL_COLOR_BUFFER_BIT,
        // real filter => fold into the cull (VISIBLE_SELECTED), not per pass
        true
    };

    m_nodeDraw->drawNodes(
        ctx,
        drawContext,
        targetBuffer);

    targetBuffer->unbind(ctx);
}

void IblProbeRenderer::enumerateProbes(const render::RenderContext& ctx)
{
    m_probes.clear();

    if (!ctx.m_collection) return;
    auto& nodes = ctx.m_collection->m_environmentProbeNodes;

    int index = 0;
    for (const auto& handle : nodes) {
        auto* node = handle.toNode();
        if (!node) continue;

        const auto* snapshot = node->getSnapshotRT();
        if (!snapshot) continue;

        auto* type = node->getType();

        ProbeMeta meta;
        meta.node = handle;
        meta.pos = snapshot->getWorldPosition();
        meta.innerRadius = type->m_environmentProbeInnerRadius;
        meta.outerRadius = type->m_environmentProbeOuterRadius;
        meta.index = index++;

        m_probes.push_back(meta);
    }
}

model::Node* IblProbeRenderer::findClosest(const render::RenderContext& ctx)
{
    if (m_probes.empty()) return nullptr;

    const glm::vec3& cameraPos = ctx.m_camera->getWorldPosition();

    const ProbeMeta* best = nullptr;
    float bestDist2 = std::numeric_limits<float>::max();

    for (const auto& probe : m_probes) {
        const float dist2 = glm::distance2(probe.pos, cameraPos);
        if (dist2 < bestDist2) {
            bestDist2 = dist2;
            best = &probe;
        }
    }

    return best ? best->node.toNode() : nullptr;
}
