#include "IblProbeRenderer.h"

#include <map>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <fmt/format.h>

#include "asset/Assets.h"
#include "asset/DynamicCubeMap.h"

#include "kigl/GLState.h"

#include "shader/Shader.h"

#include "pool/NodeHandle.h"

#include "model/Node.h"
#include "model/Snapshot.h"

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
}

void IblProbeRenderer::updateRT(const UpdateViewContext& parentCtx)
{
    if (!isEnabled()) return;

    UpdateViewContext localCtx{
        parentCtx.getEngine(),
        m_envSize,
        m_envSize };

    m_nodeDraw->updateRT(localCtx, 1.0f);
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

    model::Node* centerNode = findClosest(parentCtx);
    if (!centerNode) return false;

    // capture all 6 faces of the scene into the radiance cube. The capture's deferred pass reads
    // the IBL maps currently bound (skybox-derived this frame) => single bounce.
    for (unsigned int face = 0; face < 6; face++) {
        const auto* snapshot = centerNode->getSnapshotRT();
        if (!snapshot) continue;

        const auto& center = snapshot->getWorldPosition();
        auto& camera = m_cameras[face];
        camera.setWorldPosition(center);

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

    // mips on the captured radiance cube so prefilter importance sampling can read them
    glGenerateTextureMipmap(m_captureCube->getTextureHandle());

    const int envCubeId = static_cast<int>(m_captureCube->getTextureHandle());
    m_irradianceMap.convolve(envCubeId);
    m_prefilterMap.convolve(envCubeId);

    m_baked = true;
    m_rendered = true;
    return true;
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
        // skip the probe node itself + anything explicitly ignoring it
        [currentEntityIndex, currentId](const render::DrawableInfo& d) {
            return d.entityIndex != currentEntityIndex &&
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

model::Node* IblProbeRenderer::findClosest(const render::RenderContext& ctx)
{
    auto& nodes = ctx.m_collection->m_environmentProbeNodes;

    if (nodes.empty()) return nullptr;

    const glm::vec3& cameraPos = ctx.m_camera->getWorldPosition();

    std::map<float, model::Node*> sorted;

    for (const auto& handle : nodes) {
        auto* node = handle.toNode();
        if (!node) continue;

        const auto* snapshot = node->getSnapshotRT();
        if (!snapshot) continue;

        auto dist2 = glm::distance2(snapshot->getWorldPosition(), cameraPos);
        sorted[dist2] = node;
    }

    for (auto it = sorted.begin(); it != sorted.end(); ++it) {
        return it->second;
    }
    return nullptr;
}
