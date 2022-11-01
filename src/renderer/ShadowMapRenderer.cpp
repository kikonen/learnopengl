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

    solidShadowShader = shaders.getShader(assets, TEX_SIMPLE_DEPTH);
    blendedShadowShader = shaders.getShader(assets, TEX_SIMPLE_DEPTH, MATERIAL_COUNT, { { DEF_USE_ALPHA, "1" } });
    shadowDebugShader = shaders.getShader(assets, TEX_DEBUG_DEPTH);

    solidShadowShader->prepare(assets);
    blendedShadowShader->prepare(assets);
    shadowDebugShader->prepare(assets);

    auto buffer = new ShadowBuffer({
        assets.shadowMapSize, assets.shadowMapSize,
        { FrameBufferAttachment::getDepthTexture() } });

    shadowBuffer.reset(buffer);
    shadowBuffer->prepare(true, { 0, 0, 0, 1.0 });

    debugViewport = std::make_shared<Viewport>(
        //glm::vec3(-1 + 0.01, 1 - 0.01, 0), 
        glm::vec3(0.5, -0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f), 
        shadowBuffer->spec.attachments[0].textureID, 
        shadowDebugShader, 
        [this, &assets](Viewport& vp) {
            shadowDebugShader->nearPlane.set(assets.shadowNearPlane);
            shadowDebugShader->farPlane.set(assets.shadowFarPlane);
        });

    debugViewport->prepare(assets);
}

void ShadowMapRenderer::bind(const RenderContext& ctx)
{
    auto& node = ctx.scene->registry.m_dirLight;
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

    ctx.lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
    ctx.shadowMatrix = scaleBiasMatrix *ctx.lightSpaceMatrix;
}

void ShadowMapRenderer::bindTexture(const RenderContext& ctx)
{
    if (!rendered) return;
    shadowBuffer->bindTexture(ctx, 0, ctx.assets.shadowMapUnitIndex);
}

void ShadowMapRenderer::render(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    if (!needRender(ctx)) return;

    {
        shadowBuffer->bind(ctx);

        // NOTE KI *NO* color in shadowmap
        glClear(GL_DEPTH_BUFFER_BIT);

        ctx.useFrustum = false;
        ctx.shadow = true;
        drawNodes(ctx, registry);
        ctx.useFrustum = true;
        ctx.shadow = false;

        shadowBuffer->unbind(ctx);
    }

    rendered = true;
}

void ShadowMapRenderer::drawNodes(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    auto renderTypes = [this, &ctx](const NodeTypeMap& typeMap, ShaderBind& bound) {
        for (const auto& it : typeMap) {
            auto& type = *it.first;

            if (type.m_flags.noShadow) continue;

            Batch& batch = type.m_batch;

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
        ShaderBind bound(solidShadowShader);

        for (const auto& all : registry.solidNodes) {
            renderTypes(all.second, bound);
        }
    }

    {
        ShaderBind bound(blendedShadowShader);

        for (const auto& all : registry.alphaNodes) {
            renderTypes(all.second, bound);
        }

        for (const auto& all : registry.blendedNodes) {
            renderTypes(all.second, bound);
        }
    }
}
