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

    m_nearPlane = assets.shadowNearPlane;
    m_farPlane = assets.shadowFarPlane;

    for (int level = 0; level < assets.shadowCascades; level++) {
        auto* cascade = new ShadowCascade(level);
        m_cascades.push_back(cascade);
    }

    m_shadowDebugProgram = registry->m_programRegistry->getProgram(SHADER_DEBUG_DEPTH);
    m_shadowDebugProgram->prepare(assets);

    auto& first = m_cascades[0];

    m_debugViewport = std::make_shared<Viewport>(
        "ShadowMap",
        //glm::vec3(-1 + 0.01, 1 - 0.01, 0),
        glm::vec3(0.5, -0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        false,
        first->getTextureID(),
        m_shadowDebugProgram,
        [this, &assets](Viewport& vp) {
            u_nearPlane.set(m_nearPlane);
            u_farPlane.set(m_farPlane);
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
    m_cascades[0]->bindTexture(ctx);
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
