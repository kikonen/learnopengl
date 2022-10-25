#include "MirrorMapRenderer.h"

#include "asset/ShaderBind.h"
#include "SkyboxRenderer.h"

namespace {
    const glm::vec4 DEBUG_COLOR[6] = {
        {  1,  0,  0, 1 },
        {  0,  1,  0, 1 },
    };
}

MirrorMapRenderer::MirrorMapRenderer()
{
}

MirrorMapRenderer::~MirrorMapRenderer()
{
}

void MirrorMapRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    drawIndex = 1;

    Renderer::prepare(assets, shaders);

    drawSkip = assets.mirrorDrawSkip;

    FrameBufferSpecification spec = {
        assets.mirrorReflectionSize ,
        assets.mirrorReflectionSize,
        { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepth() }
    };

    prev = std::make_unique<TextureBuffer>(spec);
    curr = std::make_unique<TextureBuffer>(spec);

    prev->prepare(true, DEBUG_COLOR[0]);
    curr->prepare(true, DEBUG_COLOR[1]);

    debugViewport = std::make_shared<Viewport>(
        glm::vec3(-1.0, 0.5, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.5f, 0.5f),
        prev->spec.attachments[0].textureID,
        shaders.getShader(assets, TEX_VIEWPORT));

    debugViewport->prepare(assets);
    debugViewport->prepare(assets);
}

void MirrorMapRenderer::bindTexture(const RenderContext& ctx)
{
    prev->bindTexture(ctx, 0, ctx.assets.mirrorReflectionMapUnitIndex);
}

void MirrorMapRenderer::bind(const RenderContext& ctx)
{
}

void MirrorMapRenderer::render(
    const RenderContext& ctx,
    const NodeRegistry& registry,
    SkyboxRenderer* skybox)
{
    if (!stepRender()) return;

    Node* closest = findClosest(ctx, registry);
    if (!closest) return;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    const glm::vec3& planePos = closest->getWorldPos();

    // https://prideout.net/clip-planes
    // https://stackoverflow.com/questions/48613493/reflecting-scene-by-plane-mirror-in-opengl
    // reflection map
    {
        const auto mirrorSize = closest->getVolume()->getRadius() * 2;
        const auto& eyePos = ctx.camera.getPos();
        const auto& planeNormal = closest->getWorldPlaneNormal();

        const auto eyeV = planePos - eyePos;
        const auto dist = glm::length(eyeV);
        const auto eyeN = glm::normalize(eyeV);

        const auto dot = glm::dot(planeNormal, -eyeN);
        if (dot < 0) {
            // NOTE KI backside; ignore
            // => should not happen; finding closest already does this!
            return;
        }

        const auto reflectFront = glm::reflect(eyeN, planeNormal);
        const auto mirrorEyePos = planePos - (reflectFront * dist);

        //const float fovAngle = glm::degrees(2.0f * atanf((mirrorSize / 2.0f) / dist));
        const float fovAngle = 90.f;

        Camera camera(mirrorEyePos, reflectFront, ctx.camera.getViewUp());
        camera.setZoom(fovAngle);

        RenderContext localCtx("MIRROR",
            &ctx, camera,
            dist, ctx.assets.farPlane,
            curr->spec.width, curr->spec.height);
        localCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

        ClipPlaneUBO& clip = localCtx.clipPlanes.clipping[0];
        //clip.enabled = true;
        clip.plane = glm::vec4(planePos, 0);

        localCtx.bindMatricesUBO();

        curr->bind(localCtx);

        bindTexture(localCtx);
        drawNodes(localCtx, registry, skybox, closest);

        curr->unbind(ctx);
        ctx.bindClipPlanesUBO();
    }

    prev.swap(curr);

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

    ctx.bindClipPlanesUBO();
    //ctx.state.enable(GL_CLIP_DISTANCE0);
    {
        auto renderTypes = [&ctx, &current](const NodeTypeMap& typeMap) {
            ShaderBind bound(typeMap.begin()->first->m_nodeShader);

            for (const auto& it : typeMap) {
                auto& type = *it.first;

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
    //ctx.state.disable(GL_CLIP_DISTANCE0);
}

Node* MirrorMapRenderer::findClosest(const RenderContext& ctx, const NodeRegistry& registry)
{
    const glm::vec3& cameraPos = ctx.camera.getPos();
    const glm::vec3& cameraFront = ctx.camera.getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : registry.allNodes) {
        for (const auto& [type, nodes] : all.second) {
            if (!type->m_flags.mirror) continue;

            for (const auto& node : nodes) {
                const auto& planeNormal = node->getWorldPlaneNormal();

                const auto dot = glm::dot(planeNormal, -cameraFront);
                if (dot <= 0) {
                    // NOTE KI backside; ignore
                    continue;
                }

                const auto eyeV = node->getWorldPos() - cameraPos;
                sorted[glm::length(eyeV)] = node;
            }
        }
    }

    for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        return (Node*)it->second;
    }
    return nullptr;
}
