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
    drawIndex = 1;

    Renderer::prepare(assets, shaders);

    drawSkip = assets.waterDrawSkip;

    FrameBufferSpecification spec = {
        assets.waterReflectionSize , 
        assets.waterReflectionSize, 
        { FrameBufferAttachment::getTexture(), FrameBufferAttachment::getRBODepth() } 
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
    if (noiseTextureID != -1) {
        glBindTextures(ctx.assets.noiseUnitIndex, 1, &noiseTextureID);
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
    if (!stepRender()) return;

    Water* closest = findClosest(ctx, registry);
    if (!closest) return;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    glm::vec3 planePos = closest->getPos();

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

        RenderContext localCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, camera, reflectionBuffer->spec.width, reflectionBuffer->spec.height);
        localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

        ClipPlaneUBO& clip = localCtx.clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, 1, 0, -planePos.y);

        localCtx.bindMatricesUBO();

        reflectionBuffer->bind(localCtx);

        drawNodes(localCtx, registry, skybox, closest);

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

        RenderContext localCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, camera, refractionBuffer->spec.width, refractionBuffer->spec.height);
        localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

        ClipPlaneUBO& clip = localCtx.clipPlanes.clipping[0];
        clip.enabled = true;
        clip.plane = glm::vec4(0, -1, 0, planePos.y);

        localCtx.bindMatricesUBO();

        refractionBuffer->bind(localCtx);

        drawNodes(localCtx, registry, skybox, closest);

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
    Node* current)
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
        auto renderTypes = [&ctx, &current](const NodeTypeMap& typeMap) {
            ShaderBind bound(typeMap.begin()->first->defaultShader);

            for (const auto& x : typeMap) {
                auto& type = x.first;
                if (type->flags.water) continue;

                //ShaderBind bound(type->defaultShader);

                Batch& batch = type->batch;

                type->bind(ctx, bound.shader);
                batch.bind(ctx, bound.shader);

                for (auto& node : x.second) {
                    if (node == current) continue;
                    batch.draw(ctx, node, bound.shader);
                }

                batch.flush(ctx, type);
                type->unbind(ctx);
            }
        };

        for (const auto& all : registry.solidNodes) {
            renderTypes(all.second);
        }

        if (skybox) {
            skybox->render(ctx);
        }

        for (const auto& all : registry.alphaNodes) {
            renderTypes(all.second);
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
        for (const auto& x : all.second) {
            const auto& type = x.first;
            if (!type->flags.water) continue;

            for (const auto& node : x.second) {
                const glm::vec3 ray = node->getPos() - cameraPos;
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
