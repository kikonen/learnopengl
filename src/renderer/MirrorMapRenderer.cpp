#include "MirrorMapRenderer.h"

#include "asset/Assets.h"
#include "asset/Shader.h"

#include "pool/NodeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "model/Node.h"
#include "model/Viewport.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/FrameBuffer.h"
#include "render/NodeDraw.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ProgramRegistry.h"
#include "registry/NodeSnapshotRegistry.h"

#include "renderer/WaterMapRenderer.h"


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

    static const int ATT_ALBEDO_INDEX = 0;
}

void MirrorMapRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    const auto& assets = ctx.m_assets;

    m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
    m_tagMaterial.kd = glm::vec4(0.f, 0.8f, 0.f, 1.f);
    MaterialRegistry::get().registerMaterial(m_tagMaterial);

    m_renderFrameStart = assets.mirrorRenderFrameStart;
    m_renderFrameStep = assets.mirrorRenderFrameStep;

    m_nearPlane = assets.mirrorMapNearPlane;
    m_farPlane = assets.mirrorMapFarPlane;

    if (m_doubleBuffer) {
        m_bufferCount = 2;
        m_prevIndex = 1;
    }

    glm::vec3 origo(0);
    for (int i = 0; i < 1; i++) {
        auto& camera = m_cameras.emplace_back(origo, CAMERA_FRONT[i], CAMERA_UP[i]);
        camera.setFov(assets.mirrorFov);
    }

    {
        m_reflectionDebugViewport = std::make_shared<Viewport>(
            "MirrorReflect",
            glm::vec3(-1.0, 0.5, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        m_reflectionDebugViewport->setBindBefore([this](Viewport& vp) {
            auto& buffer = m_reflectionBuffers[m_prevIndex];
            vp.setTextureId(buffer->m_spec.attachments[0].textureID);
            vp.setSourceFrameBuffer(buffer.get());
            });

        m_reflectionDebugViewport->setGammaCorrect(true);
        m_reflectionDebugViewport->setHardwareGamma(true);

        m_reflectionDebugViewport->prepareRT();
    }

    m_waterMapRenderer = std::make_unique<WaterMapRenderer>(fmt::format("{}_mirror", m_name), false, false, m_squareAspectRatio);
    m_waterMapRenderer->setEnabled(assets.waterMapEnabled);

    if (m_waterMapRenderer->isEnabled()) {
        m_waterMapRenderer->prepareRT(ctx);
    }

    if (m_doubleBuffer) {
        m_mirrorMapRenderer = std::make_unique<MirrorMapRenderer>(fmt::format("{}_mirror", m_name), false, false, m_squareAspectRatio);
        m_mirrorMapRenderer->setEnabled(assets.mirrorMapEnabled);

        if (m_mirrorMapRenderer->isEnabled()) {
            m_mirrorMapRenderer->prepareRT(ctx);
        }
    }
}

void MirrorMapRenderer::updateRT(const UpdateViewContext& ctx)
{
    if (!isEnabled()) return;

    const auto& assets = ctx.m_assets;

    m_waterMapRenderer->updateRT(ctx);
    if (m_mirrorMapRenderer) {
        m_mirrorMapRenderer->updateRT(ctx);
    }

    const auto& res = ctx.m_resolution;

    int w = (int)(assets.mirrorReflectionBufferScale * res.x);
    int h = (int)(assets.mirrorReflectionBufferScale * res.y);

    if (m_squareAspectRatio) {
        h = w;
    }

    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = w != m_reflectionWidth || h != m_reflectionheight;
    if (!changed) return;

    auto albedo = render::FrameBufferAttachment::getTextureRGBHdr();
    albedo.minFilter = GL_LINEAR;
    albedo.magFilter = GL_LINEAR;
    albedo.textureWrapS = GL_REPEAT;
    albedo.textureWrapT = GL_REPEAT;

    // NOTE KI *CANNOT* share same buffer spec

    for (int i = 0; i < m_bufferCount; i++) {
        {
            render::FrameBufferSpecification spec = {
                w, h,
                {
                    albedo,
                }
            };

            m_reflectionBuffers.push_back(std::make_unique<render::FrameBuffer>(
                fmt::format("{}_mirror_reflect_{}", m_name, i),
                spec));
        }
    }

    for (auto& buf : m_reflectionBuffers) {
        buf->prepare();
    }

    m_reflectionWidth = w;
    m_reflectionheight = h;
}

void MirrorMapRenderer::bindTexture(const RenderContext& ctx)
{
    auto& reflectionBuffer = m_reflectionBuffers[m_prevIndex];

    reflectionBuffer->bindTexture(ctx, ATT_ALBEDO_INDEX, UNIT_MIRROR_REFLECTION);
}

bool MirrorMapRenderer::render(
    const RenderContext& parentCtx)
{
    parentCtx.validateRender("mirror_map");

    if (!needRender(parentCtx)) return false;

    Node* closest = findClosest(parentCtx);
    setClosest(closest, m_tagMaterial.m_registeredIndex);
    if (!closest) return false;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    // https://prideout.net/clip-planes
    // https://stackoverflow.com/questions/48613493/reflecting-scene-by-plane-mirror-in-opengl
    // reflection map
    {
        auto& camera = m_cameras[0];
        float nearPlane = 0.f;
        {
            auto& snapshotRegistry = *parentCtx.m_registry->m_activeSnapshotRegistry;

            const auto& snapshot = snapshotRegistry.getSnapshot(closest->m_snapshotIndex);
            const glm::vec3& planePos = snapshot.getWorldPosition();

            const auto* parentCamera = parentCtx.m_camera;

            const auto& volume = snapshot.getVolume();
            const glm::vec3 volumeCenter = glm::vec3(volume);
            const float volumeRadius = volume.a;

            const auto& mirrorSize = volumeRadius;
            const auto& eyePos = parentCamera->getWorldPosition();

            const auto& viewFront = glm::normalize(snapshot.getViewFront());
            const auto& viewUp = glm::normalize(snapshot.getViewUp());

            const auto eyeV = planePos - eyePos;
            const auto dist = glm::length(eyeV);
            auto eyeN = glm::normalize(eyeV);

            const auto dot = glm::dot(viewFront, parentCamera->getViewFront());
            if (dot > 0) {
                // NOTE KI backside; ignore
                // => should not happen; finding closest already does this!
                //return;
                eyeN = -eyeN;
            }

            const auto reflectFront = glm::reflect(eyeN, viewFront);
            const auto mirrorEyePos = planePos - (reflectFront * dist);

            // NOTE KI keep mirror up straight up
            //glm::vec3 reflectUp{ 0.f, 1.f, 0.f };
            //glm::vec3 reflectUp = -parentCamera->getUp();
            //glm::vec3 reflectUp = -glm::normalize(glm::cross(parentCamera->getViewRight(), reflectFront));
            glm::vec3 reflectUp = viewUp;

            //const float fovAngle = glm::degrees(2.0f * atanf((mirrorSize / 2.0f) / dist));
            //const float fovAngle = assets.mirrorFov;

            camera.setWorldPosition(mirrorEyePos);
            camera.setAxis(reflectFront, reflectUp);
            //camera.setFov(ctx.m_camera.getFov());
            //camera.setFov(fovAngle);

            nearPlane = dist + m_nearPlane;
        }

        auto& reflectionBuffer = m_reflectionBuffers[m_currIndex];

        // NOTE KI "dist" to cut-off render at mirror plane; camera is mirrored *behind* the mirror
        RenderContext localCtx("MIRROR",
            &parentCtx,
            &camera,
            nearPlane,
            m_farPlane,
            reflectionBuffer->m_spec.width,
            reflectionBuffer->m_spec.height);

        localCtx.copyShadowFrom(parentCtx);

        //ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        ////clip.enabled = true;
        //clip.plane = glm::vec4(planePos, 0);

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();

        bindTexture(localCtx);
        drawNodes(localCtx, reflectionBuffer.get(), closest);

        //ctx.updateClipPlanesUBO();
    }

    parentCtx.updateMatricesUBO();
    parentCtx.updateDataUBO();

    m_prevIndex = m_currIndex;
    m_currIndex = (m_currIndex + 1) % m_bufferCount;

    m_rendered = true;
    return true;
}

void MirrorMapRenderer::handleNodeAdded(
    Node* node)
{
    if (!isEnabled()) return;

    auto* type = node->m_typeHandle.toType();

    if (m_waterMapRenderer->isEnabled()) {
        m_waterMapRenderer->handleNodeAdded(node);
    }

    if (type->m_flags.mirror) {
        m_nodes.push_back(node->toHandle());
    }
}

void MirrorMapRenderer::drawNodes(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer,
    Node* current)
{
    const auto& assets = ctx.m_assets;

    bool renderedWater{ false };
    bool renderedMirror{ false };

    if (assets.mirrorRenderWater) {
        if (m_waterMapRenderer->isEnabled()) {
            // NOTE KI ignore mirror when not yet rendered
            m_waterMapRenderer->m_sourceNode = current;
            renderedWater = m_waterMapRenderer->render(ctx);
            m_waterMapRenderer->m_sourceNode = nullptr;
        }
    }

    if (assets.mirrorRenderMirror) {
        if (m_mirrorMapRenderer && m_mirrorMapRenderer->isEnabled()) {
            // NOTE KI ignore mirror when not yet rendered
            m_mirrorMapRenderer->m_sourceNode = current;
            renderedMirror = m_mirrorMapRenderer->render(ctx);
            m_mirrorMapRenderer->m_sourceNode = nullptr;
        }
    }

    if (m_waterMapRenderer->isEnabled() && renderedWater) {
        m_waterMapRenderer->bindTexture(ctx);
    }

    if (m_mirrorMapRenderer && m_mirrorMapRenderer->isEnabled() && renderedMirror) {
        m_mirrorMapRenderer->bindTexture(ctx);
    }

    const glm::vec4 debugColor{ 0.9f, 0.0f, 0.9f, 0.0f };
    targetBuffer->clear(ctx, GL_COLOR_BUFFER_BIT, debugColor);


    //ctx.updateClipPlanesUBO();
    //kigl::GLState::get().setEnabled(GL_CLIP_DISTANCE0, true);
    {
        Node* sourceNode = m_sourceNode.toNode();

        ctx.m_nodeDraw->drawNodes(
            ctx,
            targetBuffer,
            [](const mesh::MeshType* type) { return !type->m_flags.noReflect; },
            [current, sourceNode](const Node* node) {
                return node != current &&
                    node != sourceNode;
            },
            render::NodeDraw::KIND_ALL,
            GL_COLOR_BUFFER_BIT);
    }
    //kigl::GLState::get().setEnabled(GL_CLIP_DISTANCE0, false);
}

Node* MirrorMapRenderer::findClosest(const RenderContext& ctx)
{
    if (m_nodes.empty()) return nullptr;

    auto& snapshotRegistry = *ctx.m_registry->m_activeSnapshotRegistry;

    const auto& cameraPos = ctx.m_camera->getWorldPosition();
    const auto& cameraFront = ctx.m_camera->getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& handle : m_nodes) {
        auto* node = handle.toNode();
        if (!node) continue;

        const auto& snapshot = snapshotRegistry.getSnapshot(node->m_snapshotIndex);
        const auto& viewFront = glm::normalize(snapshot.getViewFront());

        const auto dot = glm::dot(viewFront, cameraFront);

        if (dot >= 0) {
            // NOTE KI not facing mirror; ignore
            continue;
        }

        {
            // https://stackoverflow.com/questions/59534787/signed-distance-function-3d-plane
            //const auto eyeV = node->getWorldPosition() - cameraPos;
            //const auto eyeN = glm::normalize(eyeV);
            const auto modelDot = glm::dot(viewFront, snapshot.getWorldPosition());
            const auto cameraDot = glm::dot(viewFront, cameraPos);
            const auto dot = cameraDot - modelDot;

            if (dot <= 0) {
                // NOTE KI behind mirror; ignore
                continue;
            }
        }

        //if (!node->inFrustum(ctx, 1.1f)) {
        //    // NOTE KI not in frustum; ignore
        //    continue;
        //}

        {
            const auto eyeV = snapshot.getWorldPosition() - cameraPos;
            auto dist2 = glm::distance2(snapshot.getWorldPosition(), cameraPos);

            sorted[dist2] = node;
        }
    }

    for (std::map<float, Node*>::iterator it = sorted.begin(); it != sorted.end(); ++it) {
        return (Node*)it->second;
    }
    return nullptr;
}
