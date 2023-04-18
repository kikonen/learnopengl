#include "ShadowMapRenderer.h"

#include "asset/Program.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "model/Viewport.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"

#include "registry/Registry.h"
#
#include "renderer/ShadowCascade.h"


ShadowMapRenderer::~ShadowMapRenderer()
{
    for (auto& cascade : m_cascades) {
        delete cascade;
    }
}

void ShadowMapRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);

    m_renderFrameStart = assets.shadowRenderFrameStart;
    m_renderFrameStep = assets.shadowRenderFrameStep;

    m_planes = assets.shadowPlanes;
    m_mapSizes = assets.shadowMapSizes;

    int maxCount = std::min(m_planes.size() - 1, (size_t)MAX_SHADOW_MAP_COUNT);
    for (int index = 0; index < maxCount; index++) {
        auto* cascade = new ShadowCascade(
            index,
            m_planes[index],
            m_planes[index + 1],
            m_mapSizes[index]);
        m_cascades.push_back(cascade);
    }

    m_shadowDebugProgram = registry->m_programRegistry->getProgram(SHADER_DEBUG_DEPTH);
    m_shadowDebugProgram->prepare(assets);

    for (auto& cascade : m_cascades) {
        cascade->prepare(assets, registry);
    }

    m_activeCascade = 0;

    m_debugViewport = std::make_shared<Viewport>(
        "ShadowMap",
        //glm::vec3(-1 + 0.01, 1 - 0.01, 0),
        glm::vec3(0.5, -0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        false,
        0,
        m_shadowDebugProgram);

    m_debugViewport->setBindBefore([this, &assets](Viewport& vp) {
        auto& active = m_cascades[m_activeCascade];
        vp.setTextureId(active->getTextureID());
    });

    m_debugViewport->setBindAfter([this, &assets](Viewport& vp) {
        auto& active = m_cascades[m_activeCascade];
        vp.getProgram()->u_nearPlane->set(active->m_nearPlane);
        vp.getProgram()->u_farPlane->set(active->m_farPlane);
    });

    m_debugViewport->setEffectEnabled(false);
    m_debugViewport->prepare(assets);
}

void ShadowMapRenderer::bind(const RenderContext& ctx)
{
    // NOTE KI no shadows if no light
    if (!ctx.m_useLight) return;

    auto& node = ctx.m_registry->m_nodeRegistry->m_dirLight;
    if (!node) return;

    for (auto& cascade : m_cascades) {
        cascade->bind(ctx);
    }

    ctx.m_data.u_shadowCount = m_planes.size() - 1;

    for (int i = 0; i < m_planes.size(); i++) {
        ctx.m_data.u_shadowPlanes[i] = { 0.f, 0.f, m_planes[i], 0.f };
    }
}

void ShadowMapRenderer::bindTexture(const RenderContext& ctx)
{
    if (!m_rendered) return;
    for (auto& cascade : m_cascades) {
        cascade->bindTexture(ctx);
    }
}

bool ShadowMapRenderer::render(
    const RenderContext& ctx)
{
    if (!needRender(ctx)) return false;

    // NOTE KI no shadows if no light
    if (!ctx.m_useLight) return false;

    auto& node = ctx.m_registry->m_nodeRegistry->m_dirLight;
    if (!node) return false;

    for (auto& cascade : m_cascades) {
        cascade->render(ctx);
    }

    m_rotateElapsedSecs += ctx.m_clock.elapsedSecs;
    if (m_rotateElapsedSecs > 5) {
        m_activeCascade = (m_activeCascade + 1) % m_cascades.size();
        m_rotateElapsedSecs = 0.f;
    }

    ctx.updateMatricesUBO();
    ctx.updateDataUBO();

    m_rendered = true;
    return true;
}
