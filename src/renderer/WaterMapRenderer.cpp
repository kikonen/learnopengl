#include "WaterMapRenderer.h"

#include "asset/ShaderBind.h"
#include "SkyboxRenderer.h"
#include "WaterNoiseGenerator.h"

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

    reflectionBuffer = std::make_unique<TextureBuffer>(spec);
    refractionBuffer = std::make_unique<TextureBuffer>(spec);

    reflectionBuffer->prepare(true, { 0, 0, 0, 1.0 });
    refractionBuffer->prepare(true, { 0, 0, 0, 1.0 });

    //WaterNoiseGenerator generator;
    //noiseTextureID = generator.generate();

    reflectionDebugViewport = std::make_shared<Viewport>(
        glm::vec3(0.5, 0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        reflectionBuffer->spec.attachments[0].textureID,
        shaders.getShader(assets, TEX_VIEWPORT));

    refractionDebugViewport = std::make_shared<Viewport>(
        glm::vec3(0.5, 0.0, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        refractionBuffer->spec.attachments[0].textureID,
        shaders.getShader(assets, TEX_VIEWPORT));

    reflectionDebugViewport->prepare(assets);
    refractionDebugViewport->prepare(assets);
}

void WaterMapRenderer::bindTexture(const RenderContext& ctx)
{
    if (!rendered) return;

    reflectionBuffer->bindTexture(ctx, 0, ctx.assets.waterReflectionMapUnitIndex);
    refractionBuffer->bindTexture(ctx, 0, ctx.assets.waterRefractionMapUnitIndex);
    if (noiseTextureID > 0) {
        ctx.state.bindTexture(ctx.assets.noiseUnitIndex, noiseTextureID);
    }
}

void WaterMapRenderer::bind(const RenderContext& ctx)
{
}

void WaterMapRenderer::render(
    const RenderContext& ctx,
    const NodeRegistry& registry,
    SkyboxRenderer* skybox)
{
    if (!needRender(ctx)) return;

    Water* closest = findClosest(ctx, registry);
    if (!closest) return;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    const glm::vec3& planePos = closest->getWorldPos();

    // https://prideout.net/clip-planes
    // reflection map
    {
        glm::vec3 pos = ctx.camera.getPos();
        const float dist = pos.y - planePos.y;
        pos.y -= dist * 2;

        glm::vec3 rot = ctx.camera.getRotation();
        rot.x = -rot.x;

        Camera camera(pos, ctx.camera.getFront(), ctx.camera.getUp());
        camera.setZoom(ctx.camera.getZoom());
        camera.setRotation(rot);

        RenderContext localCtx("WATER_REFLECT", &ctx, camera, reflectionBuffer->spec.width, reflectionBuffer->spec.height);
        localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

        ClipPlaneUBO& clip = localCtx.clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, 1, 0, -planePos.y);

        localCtx.bindMatricesUBO();

        reflectionBuffer->bind(localCtx);

        drawNodes(localCtx, registry, skybox, closest, true);

        reflectionBuffer->unbind(ctx);
        ctx.bindClipPlanesUBO();
    }

    // refraction map
    {
        glm::vec3 rot = ctx.camera.getRotation();
        glm::vec3 pos = ctx.camera.getPos();

        Camera camera(pos, ctx.camera.getFront(), ctx.camera.getUp());
        camera.setZoom(ctx.camera.getZoom());
        camera.setRotation(rot);

        RenderContext localCtx("WATER_REFRACT", &ctx, camera, refractionBuffer->spec.width, refractionBuffer->spec.height);
        localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

        ClipPlaneUBO& clip = localCtx.clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, -1, 0, planePos.y);

        localCtx.bindMatricesUBO();

        refractionBuffer->bind(localCtx);

        drawNodes(localCtx, registry, skybox, closest, false);

        refractionBuffer->unbind(ctx);
        ctx.bindClipPlanesUBO();
    }

    ctx.bindMatricesUBO();

    rendered = true;
}

void WaterMapRenderer::drawNodes(
    const RenderContext& ctx,
    const NodeRegistry& registry,
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
        auto renderTypes = [reflect, &ctx, &current](const NodeTypeMap& typeMap) {
            ShaderBind bound(typeMap.begin()->first->m_nodeShader);

            for (const auto& it : typeMap) {
                auto& type = *it.first;

                if (type.m_flags.water) continue;
                if (reflect && type.m_flags.noReflect) continue;
                if (!reflect && type.m_flags.noRefract) continue;

                //ShaderBind bound(type->defaultShader);

                Batch& batch = type.m_batch;

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

        for (const auto& all : registry.solidNodes) {
            renderTypes(all.second);
        }

        for (const auto& all : registry.alphaNodes) {
            renderTypes(all.second);
        }

        if (skybox) {
            skybox->render(ctx);
        }

        for (const auto& all : registry.blendedNodes) {
            renderTypes(all.second);
        }
    }
    ctx.state.disable(GL_CLIP_DISTANCE0);
}

Water* WaterMapRenderer::findClosest(
    const RenderContext& ctx,
    const NodeRegistry& registry)
{
    const glm::vec3& cameraPos = ctx.camera.getPos();
    const glm::vec3& cameraDir = ctx.camera.getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : registry.allNodes) {
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
        return (Water*)it->second;
    }

    return nullptr;
}
