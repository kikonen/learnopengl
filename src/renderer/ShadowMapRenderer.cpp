#include "ShadowMapRenderer.h"

#include "asset/Assets.h"

#include "shader/Program.h"
#include "shader/ProgramUniforms.h"
#include "shader/Shader.h"
#include "shader/Uniform.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "model/Viewport.h"

#include "engine/PrepareContext.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "renderer/ShadowCascade.h"


ShadowMapRenderer::~ShadowMapRenderer()
{
    for (auto& cascade : m_cascades) {
        delete cascade;
    }
}

void ShadowMapRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    const auto& assets = ctx.m_assets;
    auto& registry = ctx.m_registry;

    m_renderFrameStart = assets.shadowRenderFrameStart;
    m_renderFrameStep = assets.shadowRenderFrameStep;

    m_planes = assets.shadowPlanes;
    m_mapSizes = assets.shadowMapSizes;

    int maxCount = std::min(static_cast<int>(m_planes.size()) - 1, MAX_SHADOW_MAP_COUNT_ABS);
    for (int index = 0; index < maxCount; index++) {
        auto* cascade = new ShadowCascade(
            index,
            m_planes[index],
            m_planes[index + 1],
            m_mapSizes[index]);
        m_cascades.push_back(cascade);
    }

    for (auto& cascade : m_cascades) {
        cascade->prepareRT(ctx);
    }

    m_activeCascade = 0;

    {
        m_debugViewport = std::make_shared<Viewport>(
            "ShadowMap",
            //glm::vec3(-1 + 0.01, 1 - 0.01, 0),
            glm::vec3(0.5, -0.5, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_DEBUG_DEPTH));

        m_debugViewport->setBindBefore([this, &assets](Viewport& vp) {
            auto& active = m_cascades[m_activeCascade];
            vp.setTextureId(active->getTextureID());
            });

        m_debugViewport->setBindAfter([this, &assets](Viewport& vp) {
            auto& active = m_cascades[m_activeCascade];
            vp.getProgram()->m_uniforms->u_nearPlane.set(active->getNearPlane());
            vp.getProgram()->m_uniforms->u_farPlane.set(active->getFarPlane());
            });

        m_debugViewport->setEffectEnabled(false);
        m_debugViewport->prepareRT();
    }
}

void ShadowMapRenderer::bind(const RenderContext& ctx)
{
    // NOTE KI no shadows if no light
    if (!ctx.m_useLight) return;

    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;

    auto* node = nodeRegistry.getDirLightNode().toNode();
    if (!node) return;

    for (auto& cascade : m_cascades) {
        cascade->bind(ctx);
    }

    const auto size = m_planes.size() - 1;

    ctx.m_data.u_shadowCount = static_cast<int>(size);

    // NOTE KI plane defines range for cascade, u_shadowCascade_n tracks
    // *END* of cascade range (thus plane[0] is ignored here)
    ctx.m_data.u_shadowCascade_0 = size >= 1 ? m_planes[1] : 0;
    ctx.m_data.u_shadowCascade_1 = size >= 2 ? m_planes[2] : 0;
    ctx.m_data.u_shadowCascade_2 = size >= 3 ? m_planes[3] : 0;
    ctx.m_data.u_shadowCascade_3 = size >= 4 ? m_planes[4] : 0;
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
    ctx.validateRender("shadow_map");

    if (!needRender(ctx)) return false;


    // NOTE KI no shadows if no light
    if (!ctx.m_useLight) return false;

    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;

    auto* node = nodeRegistry.getDirLightNode().toNode();
    if (!node) return false;

    const auto& assets = ctx.m_assets;
    auto& state = ctx.m_state;

    {
        // OpenGL Programming Guide, 8th Edition, page 404
        // Enable polygon offset to resolve depth-fighting isuses
        state.setEnabled(GL_POLYGON_OFFSET_FILL, assets.shadowPolygonOffsetEnabled);
        state.polygonOffset(assets.shadowPolygonOffset);
        state.cullFace(GL_FRONT);

        for (auto& cascade : m_cascades) {
            cascade->render(ctx);
        }

        state.cullFace(ctx.m_defaults.m_cullFace);
        state.setEnabled(GL_POLYGON_OFFSET_FILL, false);
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
