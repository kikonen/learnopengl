#include "ShadowMapRenderer.h"

#include "asset/ShaderBind.h"
#include "scene/Scene.h"

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

void ShadowMapRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders);

    m_renderFrequency = assets.shadowRenderFrequency;

    m_solidShadowShader = shaders.getShader(assets, TEX_SIMPLE_DEPTH);
    m_blendedShadowShader = shaders.getShader(assets, TEX_SIMPLE_DEPTH, MATERIAL_COUNT, { { DEF_USE_ALPHA, "1" } });
    m_shadowDebugShader = shaders.getShader(assets, TEX_DEBUG_DEPTH);

    m_solidShadowShader->prepare(assets);
    m_blendedShadowShader->prepare(assets);
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
        m_shadowBuffer->m_spec.attachments[0].textureID,
        m_shadowDebugShader,
        [this, &assets](Viewport& vp) {
            m_shadowDebugShader->nearPlane.set(assets.shadowNearPlane);
            m_shadowDebugShader->farPlane.set(assets.shadowFarPlane);
        });

    m_debugViewport->prepare(assets);
}

void ShadowMapRenderer::bind(const RenderContext& ctx)
{
    auto& node = ctx.m_scene->m_registry.m_dirLight;
    if (!node) return;

    const glm::vec3 up{ 0.0, 1.0, 0.0 };
    const glm::mat4 lightViewMatrix = glm::lookAt(
        node->m_light->getWorldPos(),
        node->m_light->getWorldTarget(), up);

    const glm::mat4 lightProjectionMatrix = glm::ortho(
        -100.0f, 100.0f, -100.0f, 100.0f,
        ctx.assets.shadowNearPlane,
        ctx.assets.shadowFarPlane);

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
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    if (!needRender(ctx)) return;

    {
        m_shadowBuffer->bind(ctx);

        // NOTE KI *NO* color in shadowmap
        glClear(GL_DEPTH_BUFFER_BIT);

        ctx.m_useFrustum = false;
        ctx.m_shadow = true;
        drawNodes(ctx, registry);
        ctx.m_useFrustum = true;
        ctx.m_shadow = false;

        m_shadowBuffer->unbind(ctx);
    }

    m_rendered = true;
}

void ShadowMapRenderer::drawNodes(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    auto renderTypes = [this, &ctx](const NodeTypeMap& typeMap, ShaderBind& bound) {
        for (const auto& it : typeMap) {
            auto& type = *it.first;

            if (type.m_flags.noShadow) continue;

            auto& batch = ctx.m_batch;

            type.bind(ctx, bound.shader);
            batch.bind(ctx, bound.shader);

            for (auto& node : it.second) {
                batch.draw(ctx, *node, bound.shader);
            }

            batch.flush(ctx, type);
            type.unbind(ctx);
        }
    };

    {
        ShaderBind bound(m_solidShadowShader);

        for (const auto& all : registry.solidNodes) {
            renderTypes(all.second, bound);
        }
    }

    {
        ShaderBind bound(m_blendedShadowShader);

        for (const auto& all : registry.alphaNodes) {
            renderTypes(all.second, bound);
        }

        for (const auto& all : registry.blendedNodes) {
            renderTypes(all.second, bound);
        }
    }
}
