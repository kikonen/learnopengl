#include "ShadowMapRenderer.h"

#include "asset/Program.h"
#include "asset/Shader.h"

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
    m_frustumSizes = assets.shadowFrustumSizes;
    m_mapSizes = assets.shadowMapSizes;

    for (int index = 0; index < m_planes.size() - 1; index++) {
        auto* cascade = new ShadowCascade(
            index,
            m_planes[index],
            m_planes[index + 1],
            m_frustumSizes[index],
            m_mapSizes[index]);
        m_cascades.push_back(cascade);
    }

    m_shadowDebugProgram = registry->m_programRegistry->getProgram(SHADER_DEBUG_DEPTH);
    m_shadowDebugProgram->prepare(assets);

    for (auto& cascade : m_cascades) {
        cascade->prepare(assets, registry);
    }

    m_activeCascade = 0;

    auto& active = m_cascades[m_activeCascade];

    m_debugViewport = std::make_shared<Viewport>(
        "ShadowMap",
        //glm::vec3(-1 + 0.01, 1 - 0.01, 0),
        glm::vec3(0.5, -0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        false,
        active->getTextureID(),
        m_shadowDebugProgram,
        [this, &assets, &active](Viewport& vp) {
            u_nearPlane.set(active->m_nearPlane);
            u_farPlane.set(active->m_farPlane);
        });
    m_debugViewport->setEffectEnabled(false);
    m_debugViewport->prepare(assets);
}

void ShadowMapRenderer::bind(const RenderContext& ctx)
{
    auto& node = ctx.m_registry->m_nodeRegistry->m_dirLight;
    if (!node) return;

    for (auto& cascade : m_cascades) {
        cascade->bind(ctx);
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

    m_rendered = true;
    return true;
}
