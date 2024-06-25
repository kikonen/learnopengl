#include "ShadowCascade.h"

#include "util/glm_format.h"

#include "asset/Assets.h"
#include "asset/Program.h"
#include "asset/ProgramUniforms.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "component/Light.h"
#include "component/Camera.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "model/Node.h"
#include "model/Viewport.h"

#include "engine/PrepareContext.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ProgramRegistry.h"


namespace {
    // @see Computer Graphics Programmming in OpenGL Using C++, Second Edition
    // @see OpenGL Programming Guide, 8th Edition, page 406
    // The scale and bias matrix maps depth values in projection space
    // (which lie between -1.0 and +1.0) into the range 0.0 to 1.0.
    //
    // TODO KI add small bias into Translation Z component

    //auto B = glm::scale(
    //    glm::translate(
    //        glm::mat4(1),
    //        glm::vec3(0.5, 0.5, 0.5)),
    //    glm::vec3(0.5, 0.5, 0.5));
    const glm::mat4 SCALE_BIAS_MATRIX = {
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

void ShadowCascade::prepareRT(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.m_assets;
    auto& registry = ctx.m_registry;

    m_solidShadowProgram = ProgramRegistry::get().getProgram(SHADER_SIMPLE_DEPTH);
    m_alphaShadowProgram = ProgramRegistry::get().getProgram(SHADER_SIMPLE_DEPTH, { { DEF_USE_ALPHA, "1" } });

    m_solidShadowProgram->prepareRT();
    m_alphaShadowProgram->prepareRT();

    m_cascadeCount = assets.shadowPlanes.size() - 1;

    m_buffer = new render::FrameBuffer(
        fmt::format("shadow_cascade_{}", m_index),
        {
            m_mapSize, m_mapSize,
            { render::FrameBufferAttachment::getShadow() }
        });

    m_buffer->prepare();
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
    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;

    auto* node = nodeRegistry.getDirLightNode().toNode();
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

        m_camera.setWorldPosition(center - node->m_light->getWorldDirection());
        m_camera.setAxis(
            node->m_light->getWorldDirection(),
            glm::vec3{ 0.0f, 1.0f, 0.0f });

        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::min();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::min();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::min();

        for (int j = 0; j < corners.size(); j++) {
            const auto p = m_camera.getView() * corners[j];

            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
            minZ = std::min(minZ, p.z);
            maxZ = std::max(maxZ, p.z);
        }

        // Tune this parameter according to the scene
        float zMult = 5.0f; // 20.f / (m_index + 1);
        if (m_index == 0) {
            zMult = 20.f;
        } else if (m_index == 1) {
            zMult = 10.f;
        }

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

        m_camera.setViewport({ minX, maxX, minY, maxY });
        m_camera.setupProjection(1.f, minZ, maxZ);

        const float bias = 0.0f;

        const auto scaleMatrix = glm::scale(
            glm::mat4(1.f),
            glm::vec3(0.5f, 0.5f, 0.5f));

        const auto translateMatrix = glm::translate(
            glm::mat4(1.f),
            glm::vec3(0.5f, 0.5f, 0.5f - bias));

        const auto scaleBiasMatrix = translateMatrix * scaleMatrix;

        ctx.m_matrices.u_shadow[m_index] = scaleBiasMatrix * m_camera.getProjected();
        //ctx.m_matrices.u_shadow[m_index] = ctx.m_matrices.u_shadowProjected[m_index];

        //KI_INFO_OUT(fmt::format(
        //    "pos={}, viewport=({}, {}, {}, {}), frustum={}",
        //    m_camera.getWorldPosition(),
        //    minX, maxX, minY, maxY,
        //    m_camera.getFrustum().str()));

    }
}

void ShadowCascade::render(
    const RenderContext& parentCtx)
{
    RenderContext localCtx("SHADOW",
        &parentCtx,
        &m_camera,
        m_camera.getNearPlane(),
        m_camera.getFarPlane(),
        m_mapSize, m_mapSize);

    localCtx.m_defaults.m_cullFace = GL_FRONT;
    localCtx.m_shadow = true;
    localCtx.m_forceSolid = true;

    localCtx.copyShadowFrom(parentCtx);
    localCtx.updateMatricesUBO();
    localCtx.updateDataUBO();

    m_buffer->clearAll();

    m_buffer->bind(localCtx);
    drawNodes(localCtx);
}

void ShadowCascade::drawNodes(
    const RenderContext& ctx)
{
    // NOTE KI *NO* G-buffer in shadow
    const auto typeFilter = [](const mesh::MeshType* type) {
        // NOTE KI tessellation not suppported
        return !type->m_flags.noShadow;
    };

    const auto nodeFilter = [](const Node* node) {
        return true;
    };

    {
        ctx.m_nodeDraw->drawProgram(
            ctx,
            [this](const mesh::LodMesh& lodMesh) {
                if (lodMesh.m_flags.tessellation) return (Program*)nullptr;
                if (lodMesh.m_shadowProgram) return lodMesh.m_shadowProgram;
                return lodMesh.m_drawOptions.m_alpha ? m_alphaShadowProgram : m_solidShadowProgram;
            },
            typeFilter,
            nodeFilter,
            render::KIND_ALL);
    }

    ctx.m_batch->flush(ctx);
}
