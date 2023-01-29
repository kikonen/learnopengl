#include "ShadowMapRenderer.h"

#include "asset/Shader.h"

#include "component/Light.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"

namespace {
    // @see Computer Graphics Programmming in OpenGL Using C++, Second Edition
    // @see OpenGL Programming Guide, 8th Edition, page 406
    // The scale and bias matrix maps depth values in projection space
    // (which lie between -1.0 and +1.0) into the range 0.0 to 1.0.
    const glm::mat4 scaleBiasMatrix = {
      {0.5f, 0.0f, 0.0f, 0.0f},
      {0.0f, 0.5f, 0.0f, 0.0f},
      {0.0f, 0.0f, 0.5f, 0.0f},
      {0.5f, 0.5f, 0.5f, 1.0f},
    };
}

ShadowMapRenderer::ShadowMapRenderer()
{
}

ShadowMapRenderer::~ShadowMapRenderer()
{
}

void ShadowMapRenderer::prepare(
    const Assets& assets,
    ShaderRegistry& shaders,
    MaterialRegistry& materialRegistry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders, materialRegistry);

    m_renderFrameStart = assets.shadowRenderFrameStart;
    m_renderFrameStep = assets.shadowRenderFrameStep;

    m_nearPlane = assets.shadowNearPlane;
    m_farPlane = assets.shadowFarPlane;
    m_frustumSize = assets.shadowFrustumSize;

    m_shadowShader = shaders.getShader(TEX_SIMPLE_DEPTH, { { DEF_USE_ALPHA, "1" } });
    //m_solidShadowShader = shaders.getShader(TEX_SIMPLE_DEPTH);
    //m_blendedShadowShader = shaders.getShader(TEX_SIMPLE_DEPTH, { { DEF_USE_ALPHA, "1" } });
    m_shadowDebugShader = shaders.getShader(TEX_DEBUG_DEPTH);

    m_shadowShader->prepare(assets);
    //m_solidShadowShader->prepare(assets);
    //m_blendedShadowShader->prepare(assets);
    m_shadowDebugShader->prepare(assets);

    auto buffer = new ShadowBuffer({
        assets.shadowMapSize, assets.shadowMapSize,
        { FrameBufferAttachment::getDepthTexture() } });

    m_shadowBuffer.reset(buffer);
    m_shadowBuffer->prepare(true, { 0, 0, 0, 1.0 });

    m_debugViewport = std::make_shared<Viewport>(
        "ShadowMap",
        //glm::vec3(-1 + 0.01, 1 - 0.01, 0),
        glm::vec3(0.5, -0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        false,
        m_shadowBuffer->m_spec.attachments[0].textureID,
        m_shadowDebugShader,
        [this, &assets](Viewport& vp) {
            m_shadowDebugShader->u_nearPlane.set(m_nearPlane);
            m_shadowDebugShader->u_farPlane.set(m_farPlane);
        });

    m_debugViewport->prepare(assets);
}

void ShadowMapRenderer::bind(const RenderContext& ctx)
{
    auto& node = ctx.m_nodeRegistry.m_dirLight;
    if (!node) return;

    const glm::vec3 up{ 0.0, 1.0, 0.0 };
    const glm::mat4 lightViewMatrix = glm::lookAt(
        node->m_light->getWorldPosition(),
        node->m_light->getWorldTargetPosition(), up);

    const glm::mat4 lightProjectionMatrix = glm::ortho(
        -m_frustumSize, m_frustumSize, -m_frustumSize, m_frustumSize,
        m_nearPlane,
        m_farPlane);

    //lightProjection = glm::perspective(glm::radians(60.0f), (float)ctx.engine.width / (float)ctx.engine.height, near_plane, far_plane);

    ctx.m_matrices.lightProjected = lightProjectionMatrix * lightViewMatrix;
    ctx.m_matrices.shadow = scaleBiasMatrix * ctx.m_matrices.lightProjected;
}

void ShadowMapRenderer::bindTexture(const RenderContext& ctx)
{
    if (!m_rendered) return;
    m_shadowBuffer->bindTexture(ctx, 0, UNIT_SHADOW_MAP);
}

void ShadowMapRenderer::render(
    const RenderContext& ctx)
{
    if (!needRender(ctx)) return;

    // NOTE KI no shadows if no light
    if (!ctx.assets.useLight) return;

    auto& node = ctx.m_nodeRegistry.m_dirLight;
    if (!node) return;

    {
        m_shadowBuffer->bind(ctx);

        // NOTE KI *NO* color in shadowmap
        glClear(GL_DEPTH_BUFFER_BIT);

        ctx.m_shadow = true;
        ctx.m_allowBlend = false;
        drawNodes(ctx);
        ctx.m_shadow = false;
        ctx.m_allowBlend = true;

        m_shadowBuffer->unbind(ctx);
    }

    m_rendered = true;
}

void ShadowMapRenderer::drawNodes(
    const RenderContext& ctx)
{
    auto renderTypes = [this, &ctx](const MeshTypeMap& typeMap, Shader* shader) {
        for (const auto& it : typeMap) {
            auto& type = *it.first.type;
            auto& batch = ctx.m_batch;

            if (type.m_flags.noShadow) continue;

            for (auto& node : it.second) {
                batch.draw(ctx, *node, shader);
            }
        }
    };

    if (false) {
        //{
        //    auto shader = m_solidShadowShader;

        //    for (const auto& all : ctx.m_nodeRegistry.solidNodes) {
        //        renderTypes(all.second, shader);
        //    }
        //}

        //{
        //    auto shader = m_blendedShadowShader;

        //    for (const auto& all : ctx.m_nodeRegistry.alphaNodes) {
        //        renderTypes(all.second, shader);
        //    }

        //    for (const auto& all : ctx.m_nodeRegistry.blendedNodes) {
        //        renderTypes(all.second, shader);
        //    }
        //}
    }
    else {
        for (const auto& all : ctx.m_nodeRegistry.allNodes) {
            renderTypes(all.second, m_shadowShader);
        }

    }

    ctx.m_batch.flush(ctx);
}
