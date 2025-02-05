#include "WaterMapRenderer.h"

#include "asset/Assets.h"

#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "render/Camera.h"

#include "pool/NodeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "model/Node.h"
#include "model/Snapshot.h"
#include "model/Viewport.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateViewContext.h"

#include "render/FrameBuffer.h"
#include "render/NodeCollection.h"
#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/NodeDraw.h"
#include "render/DrawContext.h"

#include "WaterNoiseGenerator.h"


namespace {
    const glm::vec3 CAMERA_FRONT[2] = {
        {  0,  0,  -1 },
        {  0,  0,  -1 },
    };

    const glm::vec3 CAMERA_UP[2] = {
        {  0,  1,  0 },
        {  0,  1,  0 },
    };

    static const int ATT_ALBEDO_INDEX = 0;
}

WaterMapRenderer::~WaterMapRenderer() = default;

void WaterMapRenderer::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepareRT(ctx);

    const auto& assets = ctx.m_assets;

    {
        m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
        m_tagMaterial.registerMaterial();
    }

    m_renderFrameStart = assets.waterRenderFrameStart;
    m_renderFrameStep = assets.waterRenderFrameStep;

    m_nearPlane = assets.waterMapNearPlane;
    m_farPlane = assets.waterMapFarPlane;

    if (m_doubleBuffer) {
        m_bufferCount = 2;
        m_prevIndex = 1;
    }

    //WaterNoiseGenerator generator;
    //noiseTextureID = generator.generate();

    {
        m_reflectionDebugViewport = std::make_shared<Viewport>(
            "WaterReflect",
            glm::vec3(0.5, 0.5, 0),
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

        m_reflectionDebugViewport->prepareRT();
    }

    {
        m_refractionDebugViewport = std::make_shared<Viewport>(
            "WaterRefract",
            glm::vec3(0.5, 0.0, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        m_refractionDebugViewport->setBindBefore([this](Viewport& vp) {
            auto& buffer = m_refractionBuffers[m_prevIndex];
            vp.setTextureId(buffer->m_spec.attachments[0].textureID);
            vp.setSourceFrameBuffer(buffer.get());
            });

        m_refractionDebugViewport->prepareRT();
    }

    glm::vec3 origo(0);
    for (int i = 0; i < 2; i++) {
        auto & camera = m_cameras.emplace_back(origo, CAMERA_FRONT[i], CAMERA_UP[i]);
        camera.setFov(90.0);
    }
}

void WaterMapRenderer::updateRT(const UpdateViewContext& ctx)
{
    if (!isEnabled()) return;

    updateReflectionView(ctx);
    updateRefractionView(ctx);
}

void WaterMapRenderer::updateReflectionView(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;

    const auto& res = ctx.m_resolution;

    int w = (int)(assets.waterReflectionBufferScale * res.x);
    int h = (int)(assets.waterReflectionBufferScale * res.y);

    if (m_squareAspectRatio) {
        h = w;
    }

    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = w != m_reflectionWidth || h != m_reflectionheight;
    if (!changed) return;

    m_reflectionBuffers.clear();

    auto albedo = render::FrameBufferAttachment::getTextureRGBHdr();
    albedo.minFilter = GL_LINEAR;
    albedo.magFilter = GL_LINEAR;
    albedo.textureWrapS = GL_REPEAT;
    albedo.textureWrapT = GL_REPEAT;

    for (int i = 0; i < m_bufferCount; i++) {
        {
            render::FrameBufferSpecification spec = {
                w, h,
                {
                    albedo,
                }
            };

            m_reflectionBuffers.push_back(std::make_unique<render::FrameBuffer>(
                fmt::format("{}_water_reflect_{}", m_name, i),
                spec));
        }
    }

    for (auto& buf : m_reflectionBuffers) {
        buf->prepare();
    }

    m_reflectionWidth = w;
    m_reflectionheight = h;
}

void WaterMapRenderer::updateRefractionView(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;
    const auto& res = ctx.m_resolution;

    int w = (int)(assets.waterRefractionBufferScale * res.x);
    int h = (int)(assets.waterRefractionBufferScale * res.y);

    if (m_squareAspectRatio) {
        h = w;
    }

    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = w != m_refractionWidth || h != m_refractionHeight;
    if (!changed) return;

    m_refractionBuffers.clear();

    auto albedo = render::FrameBufferAttachment::getTextureRGBHdr();
    albedo.minFilter = GL_LINEAR;
    albedo.magFilter = GL_LINEAR;
    albedo.textureWrapS = GL_REPEAT;
    albedo.textureWrapT = GL_REPEAT;

    for (int i = 0; i < m_bufferCount; i++) {
        {
            render::FrameBufferSpecification spec = {
                w, h,
                {
                    albedo,
                }
            };

            m_refractionBuffers.push_back(std::make_unique<render::FrameBuffer>(
                fmt::format("{}_water_refract_{}", m_name, i),
                spec));
        }
    }

    for (auto& buf : m_refractionBuffers) {
        buf->prepare();
    }

    m_refractionWidth = w;
    m_refractionHeight = h;
}

void WaterMapRenderer::bindTexture(const RenderContext& ctx)
{
    //if (!m_rendered) return;

    auto& state = ctx.m_state;

    auto& refractionBuffer = m_refractionBuffers[m_prevIndex];
    auto& reflectionBuffer = m_reflectionBuffers[m_prevIndex];

    reflectionBuffer->bindTexture(ctx, ATT_ALBEDO_INDEX, UNIT_WATER_REFLECTION);
    refractionBuffer->bindTexture(ctx, ATT_ALBEDO_INDEX, UNIT_WATER_REFRACTION);

    if (m_noiseTextureID > 0) {
        state.bindTexture(UNIT_WATER_NOISE, m_noiseTextureID, false);
    }
}

bool WaterMapRenderer::render(
    const RenderContext& parentCtx)
{
    parentCtx.validateRender("water_map");

    if (!needRender(parentCtx)) return false;

    auto closest = findClosest(parentCtx);
    setClosest(closest, m_tagMaterial.m_registeredIndex);
    if (!closest) return false;

    auto& state = parentCtx.m_state;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    const auto* parentCamera = parentCtx.m_camera;
    const auto& parentCameraFront = parentCamera->getViewFront();
    const auto& parentCameraRight = parentCamera->getViewRight();
    const auto& parentCameraUp = parentCamera->getViewUp();
    const auto& parentCameraPos = parentCamera->getWorldPosition();
    const auto& parentCameraFov = parentCamera->getFov();

    const auto* snapshot = closest->getSnapshotRT();
    if (!snapshot) return false;

    const auto& planePos = snapshot->getWorldPosition();
    const float sdist = parentCameraPos.y - planePos.y;

    // https://prideout.net/clip-planes

    state.setEnabled(GL_CLIP_DISTANCE0, true);

    // reflection map
    {
        auto& camera = m_cameras[0];
        {
            glm::vec3 cameraPos = parentCameraPos;
            cameraPos.y -= sdist * 2;

            // NOTE KI rotate camera
            glm::vec3 cameraFront = parentCameraFront;
            cameraFront.y *= -1;
            glm::vec3 cameraUp = glm::normalize(glm::cross(parentCameraRight, cameraFront));

            camera.setWorldPosition(cameraPos);
            camera.setAxis(cameraFront, cameraUp);
            camera.setFov(parentCameraFov);
        }

        auto& reflectionBuffer = m_reflectionBuffers[m_currIndex];

        RenderContext localCtx(
            "WATER_REFLECT",
            &parentCtx,
            &camera,
            m_nearPlane,
            m_farPlane,
            reflectionBuffer->m_spec.width,
            reflectionBuffer->m_spec.height);

        localCtx.m_useParticles = false;
        localCtx.m_useDecals = false;
        localCtx.m_useFog = false;
        localCtx.m_useEmission = false;
        localCtx.m_useBloom = false;

        localCtx.copyShadowFrom(parentCtx);

        {
            ClipPlaneUBO& clip = localCtx.m_clipPlanes.u_clipping[0];

            glm::vec3 normal = glm::vec3(0, (sdist > 0 ? 1 : -1), 0);
            float dist = (sdist > 0 ? -1 : 1) * planePos.y;

            clip.u_plane = glm::vec4(normal, dist);
            localCtx.m_clipPlanes.u_clipCount = 1;
        }

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();
        localCtx.updateClipPlanesUBO();

        drawNodes(localCtx, reflectionBuffer.get(), closest, true);
    }

    // refraction map
    {
        auto& camera = m_cameras[1];
        {
            const auto& cameraPos = parentCameraPos;
            const auto& cameraFront = parentCameraFront;

            camera.setWorldPosition(cameraPos);
            camera.setAxis(cameraFront, parentCameraUp);
            camera.setFov(parentCameraFov);
        }

        auto& refractionBuffer = m_refractionBuffers[m_currIndex];

        RenderContext localCtx(
            "WATER_REFRACT",
            &parentCtx,
            &camera,
            m_nearPlane,
            m_farPlane,
            refractionBuffer->m_spec.width,
            refractionBuffer->m_spec.height);

        localCtx.m_useParticles = false;
        localCtx.m_useDecals = false;
        localCtx.m_useFog = false;
        localCtx.m_useEmission = false;
        localCtx.m_useBloom = false;

        localCtx.copyShadowFrom(parentCtx);

        {
            ClipPlaneUBO& clip = localCtx.m_clipPlanes.u_clipping[0];

            glm::vec3 normal = glm::vec3(0, (sdist > 0 ? -1 : 1), 0);
            float dist = (sdist > 0 ? 1 : -1) * planePos.y;

            clip.u_plane = glm::vec4(normal, dist);

            localCtx.m_clipPlanes.u_clipCount = 1;
        }

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();
        localCtx.updateClipPlanesUBO();

        drawNodes(localCtx, refractionBuffer.get(), closest, false);
    }

    parentCtx.updateMatricesUBO();
    parentCtx.updateDataUBO();
    parentCtx.updateClipPlanesUBO();

    state.setEnabled(GL_CLIP_DISTANCE0, false);

    m_prevIndex = m_currIndex;
    m_currIndex = (m_currIndex + 1) % m_bufferCount;

    m_rendered = true;

    return true;
}

void WaterMapRenderer::drawNodes(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer,
    Node* current,
    bool reflect)
{
    // NOTE KI flush before touching clip distance
    ctx.validateRender("water-nodes");

    ctx.updateClipPlanesUBO();

    const glm::vec4 debugColor(0.9f, 0.3f, 0.3f, 0.0f);
    targetBuffer->clear(ctx, GL_COLOR_BUFFER_BIT, debugColor);

    {
        Node* sourceNode = m_sourceNode.toNode();

        render::DrawContext drawContext{
            [reflect](const mesh::MeshType* type) {
                return !type->m_flags.water &&
                    (reflect ? !type->m_flags.noReflect : !type->m_flags.noRefract);
            },
            [current, sourceNode](const Node* node) {
                return node != current &&
                    node != sourceNode &&
                    node->m_ignoredBy != current->getId();
            },
            render::KIND_ALL,
            GL_COLOR_BUFFER_BIT
        };

        ctx.m_nodeDraw->drawNodes(
            ctx,
            drawContext,
            targetBuffer);
    }
}

Node* WaterMapRenderer::findClosest(
    const RenderContext& ctx)
{
    auto& nodes = ctx.m_collection->m_waterNodes;

    if (nodes.empty()) return nullptr;

    const glm::vec3& cameraPos = ctx.m_camera->getWorldPosition();
    const glm::vec3& cameraDir = ctx.m_camera->getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& handle : nodes) {
        auto* node = handle.toNode();
        if (!node) continue;

        const auto* snapshot = node->getSnapshotRT();
        if (!snapshot) continue;

        //auto dist2 = glm::distance2(snapshot.getWorldPosition(), cameraPos);
        auto dist2 = cameraPos.y - snapshot->getWorldPosition().y;

        //glm::vec3 fromCamera = glm::normalize(ray);
        //float dot = glm::dot(fromCamera, cameraDir);
        //if (dot < 0) continue;
        sorted[dist2] = node;
    }

    for (std::map<float, Node*>::iterator it = sorted.begin(); it != sorted.end(); ++it) {
        return it->second;
    }

    return nullptr;
}
