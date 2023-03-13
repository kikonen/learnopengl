#include "MirrorMapRenderer.h"

#include "asset/Shader.h"

#include "model/Node.h"
#include "model/Viewport.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"
#include "scene/TextureBuffer.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"

#include "NodeDraw.h"

namespace {
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

    const glm::vec4 DEBUG_COLOR[6] = {
        {  1,  0,  0, 1 },
        {  0,  1,  0, 1 },
    };
}


void MirrorMapRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);

    m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
    m_tagMaterial.kd = glm::vec4(0.f, 0.8f, 0.f, 1.f);
    m_registry->m_materialRegistry->add(m_tagMaterial);

    m_renderFrameStart = assets.mirrorRenderFrameStart;
    m_renderFrameStep = assets.mirrorRenderFrameStep;

    // NOTE KI *CANNOT* share same buffer spec
    {
        FrameBufferSpecification spec = {
            assets.mirrorReflectionSize ,
            assets.mirrorReflectionSize,
            { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepth() }
        };
        m_prev = std::make_unique<TextureBuffer>(spec);
    }
    {

        FrameBufferSpecification spec = {
            assets.mirrorReflectionSize ,
            assets.mirrorReflectionSize,
            { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepth() }
        };
        m_curr = std::make_unique<TextureBuffer>(spec);
    }

    m_prev->prepare(true, DEBUG_COLOR[0]);
    m_curr->prepare(true, DEBUG_COLOR[1]);

    glm::vec3 origo(0);
    for (int i = 0; i < 1; i++) {
        auto& camera = m_cameras.emplace_back(origo, CAMERA_FRONT[i], CAMERA_UP[i]);
        camera.setZoom(assets.mirrorFov);
    }

    m_debugViewport = std::make_shared<Viewport>(
        "MirrorReflect",
        glm::vec3(-1.0, 0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        true,
        0,
        m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

    m_debugViewport->prepare(assets);
}

void MirrorMapRenderer::bindTexture(const RenderContext& ctx)
{
    m_prev->bindTexture(ctx, 0, UNIT_MIRROR_REFLECTION);
}

bool MirrorMapRenderer::render(
    const RenderContext& parentCtx)
{
    if (!needRender(parentCtx)) return false;

    Node* closest = findClosest(parentCtx);
    setClosest(closest, m_tagMaterial.m_registeredIndex);
    if (!closest) return false;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    const glm::vec3& planePos = closest->getWorldPosition();

    // https://prideout.net/clip-planes
    // https://stackoverflow.com/questions/48613493/reflecting-scene-by-plane-mirror-in-opengl
    // reflection map
    {
        const auto* mainCamera = parentCtx.m_camera;

        const auto& volume = closest->getVolume();
        const glm::vec3 volumeCenter = glm::vec3(volume);
        const float volumeRadius = volume.a;

        const auto& mirrorSize = volumeRadius;
        const auto& eyePos = mainCamera->getWorldPosition();

        const auto& planeNormal = closest->getWorldPlaneNormal();

        const auto eyeV = planePos - eyePos;
        const auto dist = glm::length(eyeV);
        auto eyeN = glm::normalize(eyeV);

        const auto dot = glm::dot(planeNormal, mainCamera->getViewFront());
        if (dot > 0) {
            // NOTE KI backside; ignore
            // => should not happen; finding closest already does this!
            //return;
            eyeN = -eyeN;
        }

        const auto reflectFront = glm::reflect(eyeN, planeNormal);
        const auto mirrorEyePos = planePos - (reflectFront * dist);

        //const float fovAngle = glm::degrees(2.0f * atanf((mirrorSize / 2.0f) / dist));
        //const float fovAngle = ctx.assets.mirrorFov;

        auto& camera = m_cameras[0];
        camera.setPosition(mirrorEyePos);
        camera.setFront(reflectFront);
        camera.setUp(mainCamera->getViewUp());
        //camera.setZoom(ctx.m_camera.getZoom());
        //camera.setZoom(fovAngle);

        RenderContext localCtx("MIRROR",
            &parentCtx,
            &camera,
            dist,
            parentCtx.assets.farPlane,
            m_curr->m_spec.width, m_curr->m_spec.height);

        localCtx.m_matrices.u_lightProjected = parentCtx.m_matrices.u_lightProjected;
        localCtx.m_matrices.u_shadow = parentCtx.m_matrices.u_shadow;

        //ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        ////clip.enabled = true;
        //clip.plane = glm::vec4(planePos, 0);

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();

        bindTexture(localCtx);
        drawNodes(localCtx, m_curr.get(), closest);

        //ctx.updateClipPlanesUBO();

        m_debugViewport->setTextureId(m_curr->m_spec.attachments[0].textureID);
        m_debugViewport->setSourceFrameBuffer(m_curr.get());
    }

    m_prev.swap(m_curr);

    parentCtx.updateMatricesUBO();
    parentCtx.updateDataUBO();

    m_rendered = true;
    return true;
}

void MirrorMapRenderer::drawNodes(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    Node* current)
{
    const glm::vec4 clearColor{ 0.9f, 0.0f, 0.9f, 0.0f };

    //ctx.updateClipPlanesUBO();
    //ctx.state.enable(GL_CLIP_DISTANCE0);
    {
        ctx.m_nodeDraw->drawNodes(
            ctx,
            targetBuffer,
            true,
            [](const MeshType* type) { return !type->m_flags.noReflect; },
            [&current](const Node* node) { return node != current; },
            // NOTE KI only color & depth used
            GL_DEPTH_BUFFER_BIT,
            clearColor);
    }
    //ctx.state.disable(GL_CLIP_DISTANCE0);
}

Node* MirrorMapRenderer::findClosest(const RenderContext& ctx)
{
    const auto& cameraPos = ctx.m_camera->getWorldPosition();
    const auto& cameraFront = ctx.m_camera->getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : ctx.m_registry->m_nodeRegistry->allNodes) {
        for (const auto& [key, nodes] : all.second) {
            const auto& type = key.type;

            if (!type->m_flags.mirror) continue;

            for (const auto& node : nodes) {
                const auto& planeNormal = node->getWorldPlaneNormal();
                const auto dot = glm::dot(planeNormal, cameraFront);

                if (dot > 0) {
                    // NOTE KI not facing mirror; ignore
                    continue;
                }

                {
                    // https://stackoverflow.com/questions/59534787/signed-distance-function-3d-plane
                    //const auto eyeV = node->getWorldPosition() - cameraPos;
                    //const auto eyeN = glm::normalize(eyeV);
                    const auto planeDist = glm::dot(planeNormal, node->getWorldPosition());
                    const auto planeDot = glm::dot(planeNormal, cameraPos);
                    const auto dist = planeDot - planeDist;

                    if (dist <= 0) {
                        // NOTE KI behind mirror; ignore
                        continue;
                    }
                }

                //if (!node->inFrustum(ctx, 1.1f)) {
                //    // NOTE KI not in frustum; ignore
                //    continue;
                //}

                {
                    const auto eyeV = node->getWorldPosition() - cameraPos;
                    const auto dist = glm::length(eyeV);

                    sorted[dist] = node;
                }
            }
        }
    }

    for (std::map<float, Node*>::iterator it = sorted.begin(); it != sorted.end(); ++it) {
        return (Node*)it->second;
    }
    return nullptr;
}
