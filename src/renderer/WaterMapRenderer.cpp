#include "WaterMapRenderer.h"

#include "asset/Shader.h"

#include "component/Camera.h"

#include "model/Viewport.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ProgramRegistry.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"

#include "WaterNoiseGenerator.h"


namespace {
    const glm::vec3 CAMERA_FRONT[6] = {
        {  1,  0,  0 },
        {  1,  0,  0 },
    };

    const glm::vec3 CAMERA_UP[6] = {
        {  0,  1,  0 },
        {  0,  1,  0 },
    };

    static const int ATT_ALBEDO_INDEX = 0;
}


void WaterMapRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);

    m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
    m_registry->m_materialRegistry->add(m_tagMaterial);

    m_renderFrameStart = assets.waterRenderFrameStart;
    m_renderFrameStep = assets.waterRenderFrameStep;

    for (int i = 0; i < 2; i++) {
        {
            int size = assets.waterReflectionSize;
            int scaledSize = assets.bufferScale * size;

            FrameBufferSpecification spec = {
                scaledSize, scaledSize,
                {
                    FrameBufferAttachment::getTextureRGB(),
                }
            };

            m_reflectionBuffers.push_back(std::make_unique<FrameBuffer>("water_reflect", spec));
        }

        {
            int size = assets.waterRefractionSize;
            int scaledSize = assets.bufferScale * size;

            FrameBufferSpecification spec = {
                scaledSize, scaledSize,
                {
                    FrameBufferAttachment::getTextureRGB(),
                }
            };

            m_refractionBuffers.push_back(std::make_unique<FrameBuffer>("water_refract", spec));
        }
    }

    for (auto& buf : m_reflectionBuffers) {
        buf->prepare(true);
    }

    for (auto& buf : m_refractionBuffers) {
        buf->prepare(true);
    }

    glm::vec3 origo(0);
    for (int i = 0; i < 2; i++) {
        auto& camera = m_cameras.emplace_back(origo, CAMERA_FRONT[i], CAMERA_UP[i]);
        camera.setFov(90.0);
    }

    //WaterNoiseGenerator generator;
    //noiseTextureID = generator.generate();

    m_reflectionDebugViewport = std::make_shared<Viewport>(
        "WaterReflect",
        glm::vec3(0.5, 0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        false,
        m_reflectionBuffers[0]->m_spec.attachments[0].textureID,
        m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

    m_reflectionDebugViewport->setSourceFrameBuffer(m_reflectionBuffers[0].get());

    m_refractionDebugViewport = std::make_shared<Viewport>(
        "WaterRefract",
        glm::vec3(0.5, 0.0, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        false,
        m_refractionBuffers[0]->m_spec.attachments[0].textureID,
        m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

    m_refractionDebugViewport->setSourceFrameBuffer(m_refractionBuffers[0].get());

    m_reflectionDebugViewport->prepare(assets);
    m_refractionDebugViewport->prepare(assets);
}

void WaterMapRenderer::bindTexture(const RenderContext& ctx)
{
    //if (!m_rendered) return;

    auto& refractionBuffer = m_refractionBuffers[m_prevIndex];
    auto& reflectionBuffer = m_reflectionBuffers[m_prevIndex];

    reflectionBuffer->bindTexture(ctx, ATT_ALBEDO_INDEX, UNIT_WATER_REFLECTION);
    refractionBuffer->bindTexture(ctx, ATT_ALBEDO_INDEX, UNIT_WATER_REFRACTION);

    if (m_noiseTextureID > 0) {
        ctx.m_state.bindTexture(UNIT_WATER_NOISE, m_noiseTextureID, false);
    }
}

bool WaterMapRenderer::render(
    const RenderContext& parentCtx)
{
    if (!needRender(parentCtx)) return false;

    auto closest = findClosest(parentCtx);
    setClosest(closest, m_tagMaterial.m_registeredIndex);
    if (!closest) return false;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    const auto* parentCamera = parentCtx.m_camera;
    const auto& parentCameraFront = parentCamera->getViewFront();
    const auto& parentCameraUp = parentCamera->getViewUp();
    const auto& parentCameraPos = parentCamera->getWorldPosition();
    const auto& parentCameraFov = parentCamera->getFov();

    const auto& planePos = closest->getWorldPosition();
    const float sdist = parentCameraPos.y - planePos.y;

    // https://prideout.net/clip-planes
    // reflection map
    {
        auto& reflectionBuffer = m_reflectionBuffers[m_currIndex];

        glm::vec3 cameraPos = parentCameraPos;
        cameraPos.y -= sdist * 2;

        // NOTE KI rotate camera
        glm::vec3 cameraFront = parentCameraFront;
        cameraFront.y *= -1;

        auto& camera = m_cameras[0];
        camera.setWorldPosition(cameraPos);
        camera.setAxis(cameraFront, parentCameraUp);
        camera.setFov(parentCameraFov);

        RenderContext localCtx(
            "WATER_REFLECT", &parentCtx, &camera,
            reflectionBuffer->m_spec.width, reflectionBuffer->m_spec.height);

        localCtx.copyShadowFrom(parentCtx);

        {
            ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];

            glm::vec3 normal = glm::vec3(0, (sdist > 0 ? 1 : -1), 0);
            float dist = (sdist > 0 ? -1 : 1) * planePos.y;

            clip.plane = glm::vec4(normal, dist);
            clip.enabled = true;
        }

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();
        localCtx.updateClipPlanesUBO();

        drawNodes(localCtx, reflectionBuffer.get(), closest, true);

        m_reflectionDebugViewport->setTextureId(reflectionBuffer->m_spec.attachments[0].textureID);
        m_reflectionDebugViewport->setSourceFrameBuffer(reflectionBuffer.get());
    }

    // refraction map
    {
        auto& refractionBuffer = m_refractionBuffers[m_currIndex];

        const auto& cameraPos = parentCameraPos;
        const auto& cameraFront = parentCameraFront;

        auto& camera = m_cameras[1];
        camera.setWorldPosition(cameraPos);
        camera.setAxis(cameraFront, parentCameraUp);
        camera.setFov(parentCameraFov);

        RenderContext localCtx(
            "WATER_REFRACT", &parentCtx, &camera,
            refractionBuffer->m_spec.width, refractionBuffer->m_spec.height);

        localCtx.copyShadowFrom(parentCtx);

        {
            ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];

            glm::vec3 normal = glm::vec3(0, (sdist > 0 ? -1 : 1), 0);
            float dist = (sdist > 0 ? 1 : -1) * planePos.y;

            clip.plane = glm::vec4(normal, dist);
            clip.enabled = true;
        }

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();
        localCtx.updateClipPlanesUBO();

        drawNodes(localCtx, refractionBuffer.get(), closest, false);

        m_refractionDebugViewport->setTextureId(refractionBuffer->m_spec.attachments[0].textureID);
        m_refractionDebugViewport->setSourceFrameBuffer(refractionBuffer.get());
    }

    parentCtx.updateMatricesUBO();
    parentCtx.updateDataUBO();
    parentCtx.updateClipPlanesUBO();

    m_prevIndex = m_currIndex;
    m_currIndex = (m_currIndex + 1) % 2;

    m_rendered = true;

    return true;
}

void WaterMapRenderer::drawNodes(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    Node* current,
    bool reflect)
{
    const glm::vec4 debugColor(0.9f, 0.3f, 0.3f, 0.0f);
    targetBuffer->clear(ctx, GL_COLOR_BUFFER_BIT, debugColor);

    // NOTE KI flush before touching clip distance
    ctx.m_batch->flush(ctx);

    ctx.updateClipPlanesUBO();
    ctx.m_state.setEnabled(GL_CLIP_DISTANCE0, true);

    {
        ctx.m_nodeDraw->drawNodes(
            ctx,
            targetBuffer,
            [reflect](const MeshType* type) {
                return !type->m_flags.water &&
                    (reflect ? !type->m_flags.noReflect : !type->m_flags.noRefract);
            },
            [&current](const Node* node) { return node != current; },
            GL_COLOR_BUFFER_BIT);
    }

    ctx.m_state.setEnabled(GL_CLIP_DISTANCE0, false);
}

Node* WaterMapRenderer::findClosest(
    const RenderContext& ctx)
{
    const glm::vec3& cameraPos = ctx.m_camera->getWorldPosition();
    const glm::vec3& cameraDir = ctx.m_camera->getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : ctx.m_registry->m_nodeRegistry->allNodes) {
        for (const auto& [key, nodes] : all.second) {
            auto& type = key.type;

            if (!type->m_flags.water) continue;

            for (const auto& node : nodes) {
                const glm::vec3 ray = node->getWorldPosition() - cameraPos;
                const float distance = glm::length(ray);
                //glm::vec3 fromCamera = glm::normalize(ray);
                //float dot = glm::dot(fromCamera, cameraDir);
                //if (dot < 0) continue;
                sorted[distance] = node;
            }
        }
    }

    for (std::map<float, Node*>::iterator it = sorted.begin(); it != sorted.end(); ++it) {
        return it->second;
    }

    return nullptr;
}
