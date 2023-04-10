#include "ShadowCascade.h"

#include "util/glm_format.h"

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

    std::vector<glm::vec4> getFrustumCornersWorldSpace(
        const glm::mat4& projectedMatrix)
    {
        const auto inv = glm::inverse(projectedMatrix);

        std::vector<glm::vec4> frustumCorners;
        for (unsigned int x = 0; x < 2; ++x)
        {
            for (unsigned int y = 0; y < 2; ++y)
            {
                for (unsigned int z = 0; z < 2; ++z)
                {
                    const glm::vec4 pt =
                        inv * glm::vec4(
                            2.0f * x - 1.0f,
                            2.0f * y - 1.0f,
                            2.0f * z - 1.0f,
                            1.0f);
                    frustumCorners.push_back(pt / pt.w);
                }
            }
        }

        return frustumCorners;
    }
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

        const auto proj = glm::perspective(
            glm::radians(camera->getFov()),
            ctx.m_aspectRatio,
            m_shadowBegin,
            m_shadowEnd);

        const auto& corners = getFrustumCornersWorldSpace(proj * viewMatrix);

        glm::vec3 center = glm::vec3{ 0, 0, 0 };
        for (const auto& v : corners) {
            center += glm::vec3(v);
        }
        center /= corners.size();

        const auto shadowViewMatrix = glm::lookAt(
            center - node->m_light->getWorldDirection(),
            center,
            glm::vec3{ 0.0f, 1.0f, 0.0f });

        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::min();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::min();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::min();

        for (int j = 0; j < corners.size(); j++) {
            const auto p = shadowViewMatrix * corners[j];

            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
            minZ = std::min(minZ, p.z);
            maxZ = std::max(maxZ, p.z);
        }

        // Tune this parameter according to the scene
        constexpr float zMult = 2.0f;
        if (minZ < 0)
        {
            minZ *= zMult;
        }
        else
        {
            minZ /= zMult;
        }
        if (maxZ < 0)
        {
            maxZ /= zMult;
        }
        else
        {
            maxZ *= zMult;
        }

        const glm::mat4 shadowProjectionMatrix = glm::ortho(
            minX, maxX, minY, maxY,
            minZ,
            maxZ);

        m_nearPlane = minZ;
        m_farPlane = maxZ;

        ctx.m_matrices.u_shadowProjected[m_index] = shadowProjectionMatrix * shadowViewMatrix;
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
