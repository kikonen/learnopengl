#include "WaterMapRenderer.h"

#include "component/Camera.h"

#include "model/Viewport.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/ShaderRegistry.h"
#include "registry/MaterialRegistry.h"

#include "scene/TextureBuffer.h"
#include "scene/RenderContext.h"
#include "scene/Batch.h"

#include "SkyboxRenderer.h"
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

WaterMapRenderer::WaterMapRenderer()
{
}

WaterMapRenderer::~WaterMapRenderer()
{
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

    FrameBufferSpecification spec = {
        assets.waterReflectionSize ,
        assets.waterReflectionSize,
        { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepth() }
    };

    m_reflectionBuffer = std::make_unique<TextureBuffer>(spec);
    m_refractionBuffer = std::make_unique<TextureBuffer>(spec);

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
        m_registry->m_shaderRegistry->getShader(TEX_VIEWPORT));

    m_reflectionDebugViewport->setSourceFrameBuffer(m_reflectionBuffer.get());

    m_refractionDebugViewport = std::make_shared<Viewport>(
        "WaterRefract",
        glm::vec3(0.5, 0.0, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        true,
        m_refractionBuffer->m_spec.attachments[0].textureID,
        m_registry->m_shaderRegistry->getShader(TEX_VIEWPORT));

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
        ctx.state.bindTexture(UNIT_WATER_NOISE, m_noiseTextureID, false);
    }
}

bool WaterMapRenderer::render(
    const RenderContext& ctx,
    SkyboxRenderer* skybox)
{
    if (!needRender(ctx)) return false;

    auto closest = findClosest(ctx);
    setClosest(closest, m_tagMaterial.m_registeredIndex);
    if (!closest) return false;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    const auto& planePos = closest->getWorldPosition();

    // https://prideout.net/clip-planes
    // reflection map
    {
        const auto* mainCamera = ctx.m_camera;
        glm::vec3 pos = mainCamera->getViewPosition();
        const float dist = pos.y - planePos.y;
        pos.y -= dist * 2;

        glm::vec3 rot = ctx.m_camera->getRotation();
        rot.x = -rot.x;

        auto& camera = m_cameras[0];
        camera.setPosition(pos);
        camera.setFront(mainCamera->getFront());
        camera.setUp(mainCamera->getUp());
        camera.setZoom(mainCamera->getZoom());
        camera.setRotation(rot);

        RenderContext localCtx("WATER_REFLECT", &ctx, &camera, m_reflectionBuffer->m_spec.width, m_reflectionBuffer->m_spec.height);
        localCtx.m_matrices.lightProjected = ctx.m_matrices.lightProjected;

        ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, 1, 0, -planePos.y);

        localCtx.updateMatricesUBO();

        m_reflectionBuffer->bind(localCtx);

        drawNodes(localCtx, skybox, closest, true);

        //m_reflectionBuffer->unbind(ctx);

        ctx.updateClipPlanesUBO();
    }

    // refraction map
    {
        const auto* mainCamera = ctx.m_camera;
        const auto& rot = mainCamera->getRotation();
        const auto& pos = mainCamera->getViewPosition();

        auto& camera = m_cameras[1];
        camera.setPosition(pos);
        camera.setFront(mainCamera->getFront());
        camera.setUp(mainCamera->getUp());
        camera.setZoom(mainCamera->getZoom());
        camera.setRotation(rot);

        RenderContext localCtx("WATER_REFRACT", &ctx, &camera, m_refractionBuffer->m_spec.width, m_refractionBuffer->m_spec.height);
        localCtx.m_matrices.lightProjected = ctx.m_matrices.lightProjected;

        ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, -1, 0, planePos.y);

        localCtx.updateMatricesUBO();

        m_refractionBuffer->bind(localCtx);

        drawNodes(localCtx, skybox, closest, false);

        //m_refractionBuffer->unbind(ctx);

        ctx.updateClipPlanesUBO();
    }

    ctx.updateMatricesUBO();

    m_rendered = true;

    return true;
}

void WaterMapRenderer::drawNodes(
    const RenderContext& ctx,
    SkyboxRenderer* skybox,
    Node* current,
    bool reflect)
{
    {
        int mask = GL_DEPTH_BUFFER_BIT;
        if (ctx.assets.clearColor) {
            if (ctx.assets.debugClearColor) {
                glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
            }
            mask |= GL_COLOR_BUFFER_BIT;
        }
        glClear(mask);
    }

    ctx.updateClipPlanesUBO();
    ctx.state.enable(GL_CLIP_DISTANCE0);
    {
        auto renderTypes = [reflect, &ctx, &current](const MeshTypeMap& typeMap) {
            auto shader = typeMap.begin()->first.type->m_nodeShader;

            for (const auto& it : typeMap) {
                auto& type = *it.first.type;
                auto& batch = ctx.m_batch;

                if (type.m_flags.water) continue;

                if (reflect && type.m_flags.noReflect) continue;
                if (!reflect && type.m_flags.noRefract) continue;

                for (auto& node : it.second) {
                    if (node == current) continue;
                    batch->draw(ctx, *node, shader);
                }
            }
        };

        for (const auto& all : ctx.m_registry->m_nodeRegistry->solidNodes) {
            renderTypes(all.second);
        }

        for (const auto& all : ctx.m_registry->m_nodeRegistry->alphaNodes) {
            renderTypes(all.second);
        }

        if (skybox) {
            skybox->render(ctx);
        }

        for (const auto& all : ctx.m_registry->m_nodeRegistry->blendedNodes) {
            renderTypes(all.second);
        }
    }
    ctx.m_batch->flush(ctx);

    ctx.state.disable(GL_CLIP_DISTANCE0);
}

Node* WaterMapRenderer::findClosest(
    const RenderContext& ctx)
{
    const glm::vec3& cameraPos = ctx.m_camera->getViewPosition();
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
