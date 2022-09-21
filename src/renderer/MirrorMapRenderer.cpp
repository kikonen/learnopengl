#include "MirrorMapRenderer.h"

#include "SkyboxRenderer.h"

MirrorMapRenderer::MirrorMapRenderer()
{
}

MirrorMapRenderer::~MirrorMapRenderer()
{
}

void MirrorMapRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    Renderer::prepare(assets, shaders);

    FrameBufferSpecification spec = {
        assets.mirrorReflectionSize ,
        assets.mirrorReflectionSize,
        { FrameBufferAttachment::getTexture(), FrameBufferAttachment::getRBODepth() }
    };

    reflectionBuffer = std::make_unique<TextureBuffer>(spec);

    reflectionBuffer->prepare();

    debugViewport = std::make_shared<Viewport>(
        glm::vec3(0.5, 0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        reflectionBuffer->spec.attachments[0].textureID,
        shaders.getShader(assets, TEX_VIEWPORT));

    debugViewport->prepare(assets);
    debugViewport->prepare(assets);
}

void MirrorMapRenderer::bindTexture(const RenderContext& ctx)
{
    if (!rendered) return;

    reflectionBuffer->bindTexture(ctx, 0, ctx.assets.mirrorReflectionMapUnitIndex);
}

void MirrorMapRenderer::bind(const RenderContext& ctx)
{
}

void MirrorMapRenderer::render(const RenderContext& ctx, const NodeRegistry& registry, SkyboxRenderer* skybox)
{
    if (!stepRender()) return;

    Node* closest = findClosest(ctx, registry);
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

    ctx.bindMatricesUBO();

    rendered = true;
}

void MirrorMapRenderer::drawNodes(
    const RenderContext& ctx,
    const NodeRegistry& registry,
    SkyboxRenderer* skybox,
    Node* current)
{
    if (ctx.assets.clearColor) {
        if (ctx.assets.debugClearColor) {
            glClearColor(0.9f, 0.0f, 0.9f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    if (skybox) {
        skybox->render(ctx, registry);
    }

    ctx.bindClipPlanesUBO();
    ctx.state.enable(GL_CLIP_DISTANCE0);

    for (const auto& x : registry.nodes) {
        auto& t = x.first;
        auto shader = t->bind(ctx, nullptr);

        Batch& batch = t->batch;
        batch.bind(ctx, shader);

        for (auto& e : x.second) {
            if (e == current) continue;
            batch.draw(ctx, e, shader);
        }

        batch.flush(ctx, t);
        t->unbind(ctx);
    }

    ctx.state.disable(GL_CLIP_DISTANCE0);
}

Node* MirrorMapRenderer::findClosest(const RenderContext& ctx, const NodeRegistry& registry)
{
    const glm::vec3& cameraPos = ctx.camera.getPos();
    const glm::vec3& cameraDir = ctx.camera.getViewFront();

    std::map<float, Node*> sorted;
    for (auto& x : registry.nodes) {
        if (!x.first->flags.mirror) continue;

        for (auto& e : x.second) {
            glm::vec3 ray = e->getPos() - cameraPos;
            float distance = glm::length(ray);
            //glm::vec3 fromCamera = glm::normalize(ray);
            //float dot = glm::dot(fromCamera, cameraDir);
            //if (dot < 0) continue;
            sorted[distance] = e;
        }
    }

    for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        return (Node*)it->second;
    }
    return nullptr;
}
