#include "WaterMapRenderer.h"

#include "asset/ShaderBind.h"
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

void WaterMapRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders);

    m_renderFrequency = assets.waterRenderFrequency;

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
        m_reflectionBuffer->m_spec.attachments[0].textureID,
        shaders.getShader(assets, TEX_VIEWPORT));

    m_refractionDebugViewport = std::make_shared<Viewport>(
        "WaterRefract",
        glm::vec3(0.5, 0.0, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        m_refractionBuffer->m_spec.attachments[0].textureID,
        shaders.getShader(assets, TEX_VIEWPORT));

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

void WaterMapRenderer::render(
    const RenderContext& ctx,
    SkyboxRenderer* skybox)
{
    if (!needRender(ctx)) return;

    auto closest = findClosest(ctx);
    if (!closest) return;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    const auto& planePos = closest->getWorldPos();

    // https://prideout.net/clip-planes
    // reflection map
    {
        auto pos = ctx.m_camera.getPos();
        const float dist = pos.y - planePos.y;
        pos.y -= dist * 2;

        auto rot = ctx.m_camera.getRotation();
        rot.x = -rot.x;

        auto& camera = m_cameras[0];
        camera.setPos(pos);
        camera.setFront(ctx.m_camera.getFront());
        camera.setUp(ctx.m_camera.getUp());
        camera.setZoom(ctx.m_camera.getZoom());
        camera.setRotation(rot);

        RenderContext localCtx("WATER_REFLECT", &ctx, camera, m_reflectionBuffer->m_spec.width, m_reflectionBuffer->m_spec.height);
        localCtx.m_matrices.lightProjected = ctx.m_matrices.lightProjected;

        ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, 1, 0, -planePos.y);

        localCtx.bindMatricesUBO();

        m_reflectionBuffer->bind(localCtx);

        drawNodes(localCtx, skybox, closest, true);

        m_reflectionBuffer->unbind(ctx);
        ctx.bindClipPlanesUBO();
    }

    // refraction map
    {
        const auto& rot = ctx.m_camera.getRotation();
        const auto& pos = ctx.m_camera.getPos();

        auto& camera = m_cameras[1];
        camera.setPos(pos);
        camera.setFront(ctx.m_camera.getFront());
        camera.setUp(ctx.m_camera.getUp());
        camera.setZoom(ctx.m_camera.getZoom());
        camera.setRotation(rot);

        RenderContext localCtx("WATER_REFRACT", &ctx, camera, m_refractionBuffer->m_spec.width, m_refractionBuffer->m_spec.height);
        localCtx.m_matrices.lightProjected = ctx.m_matrices.lightProjected;

        ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, -1, 0, planePos.y);

        localCtx.bindMatricesUBO();

        m_refractionBuffer->bind(localCtx);

        drawNodes(localCtx, skybox, closest, false);

        m_refractionBuffer->unbind(ctx);
        ctx.bindClipPlanesUBO();
    }

    ctx.bindMatricesUBO();

    m_rendered = true;
}

void WaterMapRenderer::drawNodes(
    const RenderContext& ctx,
    SkyboxRenderer* skybox,
    Node* current,
    bool reflect)
{
    if (ctx.assets.clearColor) {
        if (ctx.assets.debugClearColor) {
            glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    ctx.bindClipPlanesUBO();
    ctx.state.enable(GL_CLIP_DISTANCE0);
    {
        auto renderTypes = [reflect, &ctx, &current](const MeshTypeMap& typeMap) {
            ShaderBind bound(typeMap.begin()->first->m_nodeShader);

            for (const auto& it : typeMap) {
                auto& type = *it.first;

                if (type.m_flags.water) continue;

                if (!type.m_flags.render) continue;
                if (reflect && type.m_flags.noReflect) continue;
                if (!reflect && type.m_flags.noRefract) continue;

                //ShaderBind bound(type->defaultShader);

                auto& batch = ctx.m_batch;

                type.bind(ctx, bound.shader);
                batch.bind(ctx, bound.shader);

                for (auto& node : it.second) {
                    if (node == current) continue;
                    batch.draw(ctx, *node, bound.shader);
                }

                batch.flush(ctx, type);
                type.unbind(ctx);
            }
        };

        for (const auto& all : ctx.registry.solidNodes) {
            renderTypes(all.second);
        }

        for (const auto& all : ctx.registry.alphaNodes) {
            renderTypes(all.second);
        }

        if (skybox) {
            skybox->render(ctx);
        }

        for (const auto& all : ctx.registry.blendedNodes) {
            renderTypes(all.second);
        }
    }
    ctx.state.disable(GL_CLIP_DISTANCE0);
}

Node* WaterMapRenderer::findClosest(
    const RenderContext& ctx)
{
    const glm::vec3& cameraPos = ctx.m_camera.getPos();
    const glm::vec3& cameraDir = ctx.m_camera.getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : ctx.registry.allNodes) {
        for (const auto& [type, nodes] : all.second) {
            if (!type->m_flags.water) continue;

            for (const auto& node : nodes) {
                const glm::vec3 ray = node->getWorldPos() - cameraPos;
                const float distance = glm::length(ray);
                //glm::vec3 fromCamera = glm::normalize(ray);
                //float dot = glm::dot(fromCamera, cameraDir);
                //if (dot < 0) continue;
                sorted[distance] = node;
            }
        }
    }

    for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        return it->second;
    }

    return nullptr;
}
