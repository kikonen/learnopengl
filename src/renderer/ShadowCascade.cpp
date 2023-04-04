#include "ShadowCascade.h"

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

ShadowCascade::~ShadowCascade()
{
    delete m_buffer;
}

void ShadowCascade::prepare(
    const Assets& assets,
    Registry* registry)
{
    //m_shadowProgram = m_registry->m_programRegistry->getProgram(SHADER_SIMPLE_DEPTH, { { DEF_USE_ALPHA, "1" } });
    m_solidShadowProgram = registry->m_programRegistry->getProgram(SHADER_SIMPLE_DEPTH);
    m_blendedShadowProgram = registry->m_programRegistry->getProgram(SHADER_SIMPLE_DEPTH, { { DEF_USE_ALPHA, "1" } });

    //m_shadowProgram->prepare(assets);
    m_solidShadowProgram->prepare(assets);
    m_blendedShadowProgram->prepare(assets);

    m_buffer = new FrameBuffer(
        "shadow_map",
        {
            m_mapSize, m_mapSize,
            { FrameBufferAttachment::getDepthTexture() }
        });

    m_buffer->prepare(true, { 0, 0, 0, 1.0 });
}

void ShadowCascade::bindTexture(const RenderContext& ctx)
{
    m_buffer->bindTexture(ctx, 0, UNIT_SHADOW_MAP);
}

GLuint ShadowCascade::getTextureID()
{
    return m_buffer->m_spec.attachments[0].textureID;
}

void ShadowCascade::bind(const RenderContext& ctx)
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

void ShadowCascade::render(
    const RenderContext& ctx)
{
    m_buffer->bind(ctx);

    // NOTE KI *NO* color in shadowmap
    glClear(GL_DEPTH_BUFFER_BIT);

    ctx.m_shadow = true;
    ctx.m_allowBlend = false;
    drawNodes(ctx);
    ctx.m_shadow = false;
    ctx.m_allowBlend = true;

    m_buffer->unbind(ctx);
}

void ShadowCascade::drawNodes(
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

    for (const auto& all : ctx.m_registry->m_nodeRegistry->solidNodes) {
        renderTypes(all.second, m_solidShadowProgram);
    }

    for (const auto& all : ctx.m_registry->m_nodeRegistry->alphaNodes) {
        renderTypes(all.second, m_blendedShadowProgram);
    }

    for (const auto& all : ctx.m_registry->m_nodeRegistry->blendedNodes) {
        renderTypes(all.second, m_blendedShadowProgram);
    }

    ctx.m_batch->flush(ctx);
}
