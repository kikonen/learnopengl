#include "ShadowCascade.h"

#include "asset/Program.h"
#include "asset/Shader.h"

#include "component/Light.h"
#include "component/Camera.h"

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
    // NOTE KI important, how binding works in uniforms for array
    // https://stackoverflow.com/questions/62031259/specifying-binding-for-texture-arrays-in-glsl
    m_buffer->bindTexture(ctx, 0, UNIT_SHADOW_MAP_FIRST + m_index);
}

GLuint ShadowCascade::getTextureID()
{
    return m_buffer->m_spec.attachments[0].textureID;
}

void ShadowCascade::bind(const RenderContext& ctx)
{
    auto& node = ctx.m_registry->m_nodeRegistry->m_dirLight;
    if (!node) return;

    {
        const auto& camera = ctx.m_camera;
        const auto& viewMatrix = camera->getView();
        const auto viewInverseMatrix = glm::inverse(viewMatrix);

        Camera shadowCamera{
            node->m_light->getWorldPosition(),
            node->m_light->getWorldDirection(),
            {0.f, 1.f, 0.f} };

        const auto& shadowMatrix = shadowCamera.getView();

        const float ar = ctx.m_aspectRatio;
        const float tanHalfHFOV = tanf(glm::radians(camera->getZoom() / 2.0f));
        const float tanHalfVFOV = tanf(glm::radians((camera->getZoom() * ar) / 2.0f));

        const float xn = m_shadowBegin * tanHalfHFOV;
        const float xf = m_shadowEnd * tanHalfHFOV;
        const float yn = m_shadowBegin * tanHalfVFOV;
        const float yf = m_shadowEnd * tanHalfVFOV;

        constexpr int FRUSTUM_CORNER_COUNT = 8;

        glm::vec4 frustumCorners[FRUSTUM_CORNER_COUNT] = {
            // near face
            glm::vec4(xn,   yn, m_shadowBegin, 1.0),
            glm::vec4(-xn,  yn, m_shadowBegin, 1.0),
            glm::vec4(xn,  -yn, m_shadowBegin, 1.0),
            glm::vec4(-xn, -yn, m_shadowBegin, 1.0),

            // far face
            glm::vec4(xf,   yf, m_shadowEnd, 1.0),
            glm::vec4(-xf,  yf, m_shadowEnd, 1.0),
            glm::vec4(xf,  -yf, m_shadowEnd, 1.0),
            glm::vec4(-xf, -yf, m_shadowEnd, 1.0)
        };

        glm::vec4 frustumCornersL[FRUSTUM_CORNER_COUNT]{};

        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::min();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::min();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::min();

        for (int j = 0; j < FRUSTUM_CORNER_COUNT; j++) {
            glm::vec4 vW = viewInverseMatrix * frustumCorners[j];
            frustumCornersL[j] = shadowMatrix * vW;

            minX = std::min(minX, frustumCornersL[j].x);
            maxX = std::max(maxX, frustumCornersL[j].x);
            minY = std::min(minY, frustumCornersL[j].y);
            maxY = std::max(maxY, frustumCornersL[j].y);
            minZ = std::min(minZ, frustumCornersL[j].z);
            maxZ = std::max(maxZ, frustumCornersL[j].z);
        }

        const glm::mat4 shadowProjectionMatrix = glm::ortho(
            minX, maxX, minY, maxY,
            minZ,
            maxZ);

        m_nearPlane = minZ;
        m_farPlane = maxZ;

        ctx.m_matrices.u_shadowProjected[m_index] = shadowProjectionMatrix * shadowMatrix;
        ctx.m_matrices.u_shadowPlanes[m_index] = m_shadowBegin;
        ctx.m_matrices.u_shadowPlanes[m_index + 1] = m_shadowEnd;
        ctx.m_matrices.u_shadow[m_index] = scaleBiasMatrix * ctx.m_matrices.u_shadowProjected[m_index];
    }
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

    {
        m_solidShadowProgram->bind(ctx.m_state);
        u_shadowIndex.set(m_index);

        for (const auto& all : ctx.m_registry->m_nodeRegistry->solidNodes) {
            renderTypes(all.second, m_solidShadowProgram);
        }
    }

    {
        m_blendedShadowProgram->bind(ctx.m_state);
        u_shadowIndex.set(m_index);

        for (const auto& all : ctx.m_registry->m_nodeRegistry->alphaNodes) {
            renderTypes(all.second, m_blendedShadowProgram);
        }

        for (const auto& all : ctx.m_registry->m_nodeRegistry->blendedNodes) {
            renderTypes(all.second, m_blendedShadowProgram);
        }
    }

    ctx.m_batch->flush(ctx);
}
