#include "ShadowMapRenderer.h"

#include "asset/Program.h"
#include "asset/Shader.h"

#include "component/Light.h"

#include "model/Viewport.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


namespace {
    // @see Computer Graphics Programmming in OpenGL Using C++, Second Edition
    // @see OpenGL Programming Guide, 8th Edition, page 406
    // The scale and bias matrix maps depth values in projection space
    // (which lie between -1.0 and +1.0) into the range 0.0 to 1.0.
    //
    // TODO KI add small bias into Translation Z component
    const glm::mat4 scaleBiasMatrix = {
      {0.5f, 0.0f, 0.0f, 0.0f},
      {0.0f, 0.5f, 0.0f, 0.0f},
      {0.0f, 0.0f, 0.5f, 0.0f},
      {0.5f, 0.5f, 0.5f, 1.0f},
    };
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
    m_frustumSize = assets.shadowFrustumSize;

    m_shadowProgram = m_registry->m_programRegistry->getProgram(SHADER_SIMPLE_DEPTH, { { DEF_USE_ALPHA, "1" } });
    //m_solidShadowProgram = programs.getProgram(SHADER_SIMPLE_DEPTH);
    //m_blendedShadowProgram = programs.getProgram(SHADER_SIMPLE_DEPTH, { { DEF_USE_ALPHA, "1" } });
    m_shadowDebugProgram = m_registry->m_programRegistry->getProgram(SHADER_DEBUG_DEPTH);

    m_shadowProgram->prepare(assets);
    //m_solidShadowProgram->prepare(assets);
    //m_blendedShadowProgram->prepare(assets);
    m_shadowDebugProgram->prepare(assets);

    auto buffer = new FrameBuffer({
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

    const glm::vec3 up{ 0.0, 1.0, 0.0 };
    const glm::mat4 lightViewMatrix = glm::lookAt(
        node->m_light->getWorldPosition(),
        node->m_light->getWorldTargetPosition(), up);

    const glm::mat4 lightProjectionMatrix = glm::ortho(
        -m_frustumSize, m_frustumSize, -m_frustumSize, m_frustumSize,
        m_nearPlane,
        m_farPlane);

    //lightProjection = glm::perspective(glm::radians(60.0f), (float)ctx.engine.width / (float)ctx.engine.height, near_plane, far_plane);

    ctx.m_matrices.u_lightProjected = lightProjectionMatrix * lightViewMatrix;
    ctx.m_matrices.u_shadow = scaleBiasMatrix * ctx.m_matrices.u_lightProjected;
}

void ShadowMapRenderer::bindTexture(const RenderContext& ctx)
{
    if (!m_rendered) return;
    m_shadowBuffer->bindTexture(ctx, 0, UNIT_SHADOW_MAP);
}

bool ShadowMapRenderer::render(
    const RenderContext& ctx)
{
    if (!needRender(ctx)) return false;

    // NOTE KI no shadows if no light
    if (!ctx.m_useLight) return false;

    auto& node = ctx.m_registry->m_nodeRegistry->m_dirLight;
    if (!node) return false;

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
    return true;
}

void ShadowMapRenderer::drawNodes(
    const RenderContext& ctx)
{
    // NOTE KI *NO* G-buffer in shadow
    auto renderTypes = [this, &ctx](const MeshTypeMap& typeMap, Program* program) {
        for (const auto& it : typeMap) {
            auto* type = it.first.type;
            auto& batch = ctx.m_batch;

            if (type->m_flags.noShadow) continue;

            // NOTE KI tessellation not suppported
            if (type->m_flags.tessellation) continue;

            // NOTE KI point sprite currently not supported
            if (type->m_entityType == EntityType::sprite) continue;

            for (auto& node : it.second) {
                batch->draw(ctx, *node, program);
            }
        }
    };

    for (const auto& all : ctx.m_registry->m_nodeRegistry->allNodes) {
        renderTypes(all.second, m_shadowProgram);
    }

    ctx.m_batch->flush(ctx);
}
