#include "MirrorMapRenderer.h"

#include "asset/Assets.h"

#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "pool/NodeHandle.h"

#include "mesh/LodMesh.h"

#include "model/Node.h"
#include "model/Snapshot.h"
#include "model/Viewport.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/NodeCollection.h"
#include "debug/DebugContext.h"
#include "render/Batch.h"
#include "render/FrameBuffer.h"
#include "render/NodeDraw.h"
#include "render/DrawContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

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

    constexpr int ATT_ALBEDO_INDEX = 0;
}

MirrorMapRenderer::~MirrorMapRenderer() = default;

void MirrorMapRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    {
        m_nodeDraw = std::make_unique<render::NodeDraw>(m_name);

        auto& pipeline = m_nodeDraw->m_pipeline;
        pipeline.m_particle = false;
        pipeline.m_decal = false;
        pipeline.m_fog = false;
        pipeline.m_emission = false;
        pipeline.m_bloom = false;

        m_nodeDraw->prepareRT(ctx);
    }

    const auto& assets = ctx.getAssets();

    {
        m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
        m_tagMaterial.kd = glm::vec4(0.f, 0.8f, 0.f, 1.f);
        m_tagMaterial.registerMaterial();
    }

    m_renderFrameStart = assets.mirrorMapRenderFrameStart;
    m_renderFrameStep = assets.mirrorMapRenderFrameStep;

    m_nearPlane = assets.mirrorMapNearPlane;
    m_farPlane = assets.mirrorMapFarPlane;

    if (m_doubleBuffer) {
        m_bufferCount = 2;
        m_prevIndex = 1;
    }

    glm::vec3 origo(0);
    for (int i = 0; i < 1; i++) {
        auto& camera = m_cameras.emplace_back(origo, CAMERA_FRONT[i], CAMERA_UP[i]);
        camera.setFov(assets.mirrorMapFov);
    }

    {
        m_reflectionDebugViewport = std::make_shared<model::Viewport>(
            "MirrorReflect",
            glm::vec3(-1.0, 0.5, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        m_reflectionDebugViewport->setBindBefore([this](model::Viewport& vp) {
            auto& buffer = m_reflectionBuffers[m_prevIndex];
            vp.setTexture(
                buffer->m_spec.attachments[0].textureID,
                buffer->m_spec.getSize());
            vp.setSourceFrameBuffer(buffer.get());
            });

        m_reflectionDebugViewport->prepareRT();
    }

    if (m_doubleBuffer) {
        m_waterMapRenderer = std::make_unique<WaterMapRenderer>(fmt::format("{}_mirror", m_name), false, false, m_squareAspectRatio);
        m_waterMapRenderer->setEnabled(assets.waterMapEnabled);

        if (m_waterMapRenderer->isEnabled()) {
            m_waterMapRenderer->prepareRT(ctx);
        }
    }

    if (m_doubleBuffer) {
        m_mirrorMapRenderer = std::make_unique<MirrorMapRenderer>(fmt::format("{}_mirror", m_name), false, false, m_squareAspectRatio);
        m_mirrorMapRenderer->setEnabled(assets.mirrorMapEnabled);

        if (m_mirrorMapRenderer->isEnabled()) {
            m_mirrorMapRenderer->prepareRT(ctx);
        }
    }
}

void MirrorMapRenderer::updateRT(const UpdateViewContext& parentCtx)
{
    const auto& dbg = parentCtx.getDebug();
    m_enabled = dbg.m_mirrorMapEnabled;

    if (!isEnabled()) return;

    for (int i = 0; i < 1; i++) {
        auto& camera = m_cameras[i];
        camera.setFov(dbg.m_mirrorMapFov);
    }

    const auto& assets = parentCtx.getAssets();

    int w;
    int h;
    {
        const auto& res = parentCtx.m_resolution;

        w = (int)(dbg.m_mirrorMapReflectionBufferScale * res.x);
        h = (int)(dbg.m_mirrorMapReflectionBufferScale * res.y);

        if (m_squareAspectRatio) {
            h = w;
        }

        if (w < 1) w = 1;
        if (h < 1) h = 1;

        bool changed = w != m_reflectionWidth || h != m_reflectionheight;
        if (!changed) return;

        m_reflectionWidth = w;
        m_reflectionheight = h;
    }

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
                fmt::format("{}_reflect_{}x{}_{}", m_name, w, h, i),
                spec));
        }
    }

    for (auto& buf : m_reflectionBuffers) {
        buf->prepare();
    }

    {
        UpdateViewContext localCtx{
            parentCtx.getEngine(),
            w,
            h};
        m_nodeDraw->updateRT(localCtx, 1.f);

        // NOTE KI nested renderers scale down from current
        if (m_waterMapRenderer) {
            m_waterMapRenderer->updateRT(localCtx);
        }
        if (m_mirrorMapRenderer) {
            m_mirrorMapRenderer->updateRT(localCtx);
        }
    }
}

void MirrorMapRenderer::bindTexture(kigl::GLState& state)
{
    auto& reflectionBuffer = m_reflectionBuffers[m_prevIndex];

    if (!isEnabled()) {
        reflectionBuffer->unbindTexture(state, UNIT_MIRROR_REFLECTION);
        return;
    }

    reflectionBuffer->bindTexture(state, ATT_ALBEDO_INDEX, UNIT_MIRROR_REFLECTION);
}

bool MirrorMapRenderer::render(
    const render::RenderContext& parentCtx)
{
    parentCtx.validateRender("mirror_map");

    if (!needRender(parentCtx)) return false;

    model::Node* closest = findClosest(parentCtx);
    setClosest(parentCtx, closest, m_tagMaterial.m_registeredIndex);
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
            const auto* snapshot = closest->getSnapshotRT();
            if (!snapshot) return false;

            const auto& planePos = snapshot->getWorldPosition();

            const auto* parentCamera = parentCtx.m_camera;

            const auto& worldVolume = snapshot->getWorldVolume();
            const auto& volumeCenter = worldVolume.getCenter();
            const float volumeRadius = worldVolume.radius;

            const auto& mirrorSize = volumeRadius;
            const auto& eyePos = parentCamera->getWorldPosition();

            const auto& viewFront = glm::normalize(snapshot->getViewFront());
            const auto& viewUp = glm::normalize(snapshot->getViewUp());

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
        render::RenderContext localCtx("MIRROR",
            &parentCtx,
            &camera,
            nearPlane,
            m_farPlane,
            reflectionBuffer->m_spec.width,
            reflectionBuffer->m_spec.height);

        localCtx.m_useSsao = false;
        localCtx.m_useParticles = false;
        localCtx.m_useDecals = false;
        localCtx.m_useFog = false;
        localCtx.m_useEmission = false;
        localCtx.m_useBloom = false;
        localCtx.m_forceLineMode = false;

        //ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        ////clip.enabled = true;
        //clip.plane = glm::vec4(planePos, 0);

        bindTexture(localCtx.getGLState());
        drawNodes(localCtx, reflectionBuffer.get(), closest);

        //ctx.updateClipPlanesUBO();
    }

    m_prevIndex = m_currIndex;
    m_currIndex = (m_currIndex + 1) % m_bufferCount;

    m_rendered = true;
    return true;
}

void MirrorMapRenderer::drawNodes(
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer,
    model::Node* current)
{
    const auto& assets = ctx.getAssets();
    const auto& dbg = ctx.getDebug();

    bool renderedWater{ false };
    bool renderedMirror{ false };

    if (dbg.m_mirrorMapRenderWater) {
        if (m_waterMapRenderer && m_waterMapRenderer->isEnabled()) {
            // NOTE KI ignore mirror when not yet rendered
            m_waterMapRenderer->m_sourceNode = current;
            renderedWater = m_waterMapRenderer->render(ctx);
            m_waterMapRenderer->m_sourceNode = nullptr;
        }
    }

    if (dbg.m_mirrorMapRenderMirror) {
        if (m_mirrorMapRenderer && m_mirrorMapRenderer->isEnabled()) {
            // NOTE KI ignore mirror when not yet rendered
            m_mirrorMapRenderer->m_sourceNode = current;
            renderedMirror = m_mirrorMapRenderer->render(ctx);
            m_mirrorMapRenderer->m_sourceNode = nullptr;
        }
    }

    // NOTE KI memory barrier to ensure nested renders complete before
    // binding their textures for reading in the main mirror render
    if (renderedWater || renderedMirror) {
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    if (m_waterMapRenderer && m_waterMapRenderer->isEnabled() /*&& renderedWater*/) {
        m_waterMapRenderer->bindTexture(ctx.getGLState());
    }

    if (m_mirrorMapRenderer && m_mirrorMapRenderer->isEnabled() /*&& renderedMirror*/) {
        m_mirrorMapRenderer->bindTexture(ctx.getGLState());
    }

    ctx.updateUBOs();
    ctx.bindDefaults();

    //const glm::vec4 debugColor{ 0.9f, 0.0f, 0.9f, 0.0f };
    //targetBuffer->clear(ctx, GL_COLOR_BUFFER_BIT, debugColor);
    targetBuffer->clearAll();

    //ctx.updateClipPlanesUBO();
    //kigl::GLState::get().setEnabled(GL_CLIP_DISTANCE0, true);
    {
        model::Node* sourceNode = m_sourceNode.toNode();

        render::DrawContext drawContext{
            [current, sourceNode](const model::Node* node) {
                return !node->m_typeFlags.noReflect &&
                    node != current &&
                    node != sourceNode &&
                    node->m_ignoredBy != current->getId();
            },
            render::KIND_ALL,
            GL_COLOR_BUFFER_BIT
        };

        m_nodeDraw->drawNodes(
            ctx,
            drawContext,
            targetBuffer);
    }
    //kigl::GLState::get().setEnabled(GL_CLIP_DISTANCE0, false);
}

model::Node* MirrorMapRenderer::findClosest(const render::RenderContext& ctx)
{
    auto& nodes = ctx.m_collection->m_mirrorNodes;

    if (nodes.empty()) return nullptr;

    const auto& cameraPos = ctx.m_camera->getWorldPosition();
    const auto& cameraFront = ctx.m_camera->getViewFront();

    std::map<float, model::Node*> sorted;

    for (const auto& handle : nodes) {
        auto* node = handle.toNode();
        if (!node) continue;

        const auto* snapshot = node->getSnapshotRT();
        if (!snapshot) continue;

        const auto& viewFront = glm::normalize(snapshot->getViewFront());

        const auto dot = glm::dot(viewFront, cameraFront);

        if (dot >= 0) {
            // NOTE KI not facing mirror; ignore
            continue;
        }

        {
            // https://stackoverflow.com/questions/59534787/signed-distance-function-3d-plane
            //const auto eyeV = node->getWorldPosition() - cameraPos;
            //const auto eyeN = glm::normalize(eyeV);
            const auto modelDot = glm::dot(viewFront, snapshot->getWorldPosition());
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
            const auto eyeV = snapshot->getWorldPosition() - cameraPos;
            auto dist2 = glm::distance2(snapshot->getWorldPosition(), cameraPos);

            sorted[dist2] = node;
        }
    }

    for (std::map<float, model::Node*>::iterator it = sorted.begin(); it != sorted.end(); ++it) {
        return (model::Node*)it->second;
    }
    return nullptr;
}
