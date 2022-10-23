#include "ShadowMapRenderer.h"

#include "asset/ShaderBind.h"
#include "scene/Scene.h"

ShadowMapRenderer::ShadowMapRenderer()
{
    drawIndex = 1;
}

ShadowMapRenderer::~ShadowMapRenderer()
{
}

void ShadowMapRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders);

    drawSkip = assets.shadowDrawSkip;

    solidShadowShader = shaders.getShader(assets, TEX_SIMPLE_DEPTH);
    blendedShadowShader = shaders.getShader(assets, TEX_SIMPLE_DEPTH, { DEF_USE_ALPHA });
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

    //glm::mat4 b = {
    //    {0.5f, 0.0f, 0.0f, 0.0f},
    //    {0.0f, 0.5f, 0.0f, 0.0f},
    //    {0.0f, 0.0f, 0.5f, 0.0f},
    //    {0.5f, 0.5f, 0.5f, 1.0f},
    //};

    const glm::vec3 up{ 0.0, 1.0, 0.0 };
    const glm::mat4 lightView = glm::lookAt(
        node->m_light->getWorldPos(),
        node->m_light->getWorldTarget(), up);

    const glm::mat4 lightProjection = glm::ortho(
        -100.0f, 100.0f, -100.0f, 100.0f,
        ctx.assets.shadowNearPlane,
        ctx.assets.shadowFarPlane);

    //lightProjection = glm::perspective(glm::radians(60.0f), (float)ctx.engine.width / (float)ctx.engine.height, near_plane, far_plane);

    ctx.lightSpaceMatrix = lightProjection * lightView;
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
    if (!stepRender()) return;

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
