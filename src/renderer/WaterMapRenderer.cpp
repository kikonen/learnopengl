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

    {
        int size = assets.waterReflectionSize;
        int scaledSize = assets.bufferScale * size;

        FrameBufferSpecification spec = {
            scaledSize, scaledSize,
            { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepth() }
        };

        m_reflectionBuffer = std::make_unique<FrameBuffer>(spec);
    }

    {
        int size = assets.waterRefractionSize;
        int scaledSize = assets.bufferScale * size;

        FrameBufferSpecification spec = {
            scaledSize, scaledSize,
            { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepth() }
        };

        m_refractionBuffer = std::make_unique<FrameBuffer>(spec);
    }

    m_reflectionBuffer->prepare(true, { 0, 0, 0, 1.0 });
    m_refractionBuffer->prepare(true, { 0, 0, 0, 1.0 });

    glm::vec3 origo(0);
    for (int i = 0; i < 2; i++) {
        auto& camera = m_cameras.emplace_back(origo, CAMERA_FRONT[i], CAMERA_UP[i]);
        camera.setZoom(90.0);
    }

    //WaterNoiseGenerator generator;
    //noiseTextureID = generator.generate();

    m_reflectionDebugViewport = std::make_shared<Viewport>(
        "WaterReflect",
        glm::vec3(0.5, 0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        true,
        m_reflectionBuffer->m_spec.attachments[0].textureID,
        m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

    m_reflectionDebugViewport->setSourceFrameBuffer(m_reflectionBuffer.get());

    m_refractionDebugViewport = std::make_shared<Viewport>(
        "WaterRefract",
        glm::vec3(0.5, 0.0, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        true,
        m_refractionBuffer->m_spec.attachments[0].textureID,
        m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

    m_refractionDebugViewport->setSourceFrameBuffer(m_refractionBuffer.get());

    m_reflectionDebugViewport->prepare(assets);
    m_refractionDebugViewport->prepare(assets);
}

void WaterMapRenderer::bindTexture(const RenderContext& ctx)
{
    if (!m_rendered) return;

    m_reflectionBuffer->bindTexture(ctx, 0, UNIT_WATER_REFLECTION);
    m_refractionBuffer->bindTexture(ctx, 0, UNIT_WATER_REFRACTION);
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

    const auto* mainCamera = parentCtx.m_camera;
    const auto& cameraRot = mainCamera->getRotation();
    const auto& cameraPos = mainCamera->getWorldPosition();

    const auto& planePos = closest->getWorldPosition();
    const float sdist = cameraPos.y - planePos.y;

    // https://prideout.net/clip-planes
    // reflection map
    {
        glm::vec3 pos = cameraPos;
        pos.y -= sdist * 2;

        glm::vec3 rot = cameraRot;
        rot.x = -rot.x;

        auto& camera = m_cameras[0];
        camera.setWorldPosition(pos);
        camera.setAxis(mainCamera->getFront(), mainCamera->getUp());
        camera.setZoom(mainCamera->getZoom());
        camera.setRotation(rot);

        RenderContext localCtx(
            "WATER_REFLECT", &parentCtx, &camera,
            m_reflectionBuffer->m_spec.width, m_reflectionBuffer->m_spec.height);

        localCtx.m_matrices.u_lightProjected = parentCtx.m_matrices.u_lightProjected;
        localCtx.m_matrices.u_shadow = parentCtx.m_matrices.u_shadow;

        ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, (sdist > 0 ? 1 : 1), 0, (sdist > 0 ? -1 : 1) * planePos.y);

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();

        drawNodes(localCtx, m_reflectionBuffer.get(), closest, true);

        //m_reflectionBuffer->unbind(ctx);

        parentCtx.updateClipPlanesUBO();
        parentCtx.updateDataUBO();
    }

    // refraction map
    {
        //glm::vec3 pos = cameraPos;
        //glm::vec3 rot = cameraRot;

        auto& camera = m_cameras[1];
        camera.setWorldPosition(cameraPos);
        camera.setAxis(mainCamera->getFront(), mainCamera->getUp());
        camera.setZoom(mainCamera->getZoom());
        camera.setRotation(cameraRot);

        RenderContext localCtx(
            "WATER_REFRACT", &parentCtx, &camera,
            m_refractionBuffer->m_spec.width, m_refractionBuffer->m_spec.height);

        localCtx.m_matrices.u_lightProjected = parentCtx.m_matrices.u_lightProjected;
        localCtx.m_matrices.u_shadow = parentCtx.m_matrices.u_shadow;

        // TODO KI refraction plane clipping produces inproper clipping
        ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, (sdist > 0 ? 1 : 1), 0, (sdist > 0 ? 1 : -1) * planePos.y);

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();

        drawNodes(localCtx, m_refractionBuffer.get(), closest, false);

        parentCtx.updateClipPlanesUBO();
    }

    parentCtx.updateMatricesUBO();
    parentCtx.updateDataUBO();

    m_rendered = true;

    return true;
}

void WaterMapRenderer::drawNodes(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    Node* current,
    bool reflect)
{
    const glm::vec4 clearColor(0.9f, 0.3f, 0.3f, 0.0f);

    // NOTE KI flush before touching clip distance
    ctx.m_batch->flush(ctx);

    ctx.updateClipPlanesUBO();
    ctx.m_state.enable(GL_CLIP_DISTANCE0);

    {
        ctx.m_nodeDraw->drawNodes(
            ctx,
            targetBuffer,
            true,
            [reflect](const MeshType* type) {
                return !type->m_flags.water &&
                    (reflect ? !type->m_flags.noReflect : !type->m_flags.noRefract);
            },
            [&current](const Node* node) { return node != current; },
            // NOTE KI only color & depth used
            GL_DEPTH_BUFFER_BIT,
            clearColor);
    }

    ctx.m_state.disable(GL_CLIP_DISTANCE0);
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
