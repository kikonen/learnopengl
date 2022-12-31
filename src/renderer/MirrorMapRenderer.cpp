#include "MirrorMapRenderer.h"

#include "SkyboxRenderer.h"

#include "registry/MaterialRegistry.h"

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

MirrorMapRenderer::MirrorMapRenderer()
{
}

MirrorMapRenderer::~MirrorMapRenderer()
{
}

void MirrorMapRenderer::prepare(
    const Assets& assets,
    ShaderRegistry& shaders,
    MaterialRegistry& materialRegistry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders, materialRegistry);

    m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
    m_tagMaterial.kd = glm::vec4(0.f, 0.8f, 0.f, 1.f);
    materialRegistry.add(m_tagMaterial);

    m_renderFrequency = assets.mirrorRenderFrequency;

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
        -1,
        shaders.getShader(assets, TEX_VIEWPORT));

    m_debugViewport->prepare(assets);
}

void MirrorMapRenderer::bindTexture(const RenderContext& ctx)
{
    m_prev->bindTexture(ctx, 0, UNIT_MIRROR_REFLECTION);
}

void MirrorMapRenderer::render(
    const RenderContext& ctx,
    SkyboxRenderer* skybox)
{
    if (!needRender(ctx)) return;

    Node* closest = findClosest(ctx);
    if (!closest) return;

    closest->m_tagMaterialIndex = m_tagMaterial.m_registeredIndex;

    // https://www.youtube.com/watch?v=7T5o4vZXAvI&list=PLRIWtICgwaX23jiqVByUs0bqhnalNTNZh&index=7
    // computergraphicsprogrammminginopenglusingcplusplussecondedition.pdf

    const glm::vec3& planePos = closest->getWorldPos();

    // https://prideout.net/clip-planes
    // https://stackoverflow.com/questions/48613493/reflecting-scene-by-plane-mirror-in-opengl
    // reflection map
    {
        auto& mainCamera = ctx.m_camera;
        const auto& mirrorSize = closest->getVolume()->getRadius() * 2;
        const auto& eyePos = mainCamera.getPos();
        const auto& planeNormal = closest->getWorldPlaneNormal();

        const auto eyeV = planePos - eyePos;
        const auto dist = glm::length(eyeV);
        auto eyeN = glm::normalize(eyeV);

        const auto dot = glm::dot(planeNormal, mainCamera.getViewFront());
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
        camera.setPos(mirrorEyePos);
        camera.setFront(reflectFront);
        camera.setUp(ctx.m_camera.getViewUp());
        //camera.setZoom(ctx.m_camera.getZoom());
        //camera.setZoom(fovAngle);

        RenderContext localCtx("MIRROR",
            &ctx, camera,
            dist,
            ctx.assets.farPlane,
            m_curr->m_spec.width, m_curr->m_spec.height);
        localCtx.m_matrices.lightProjected = ctx.m_matrices.lightProjected;

        ClipPlaneUBO& clip = localCtx.m_clipPlanes.clipping[0];
        //clip.enabled = true;
        clip.plane = glm::vec4(planePos, 0);

        localCtx.bindMatricesUBO();

        m_curr->bind(localCtx);

        bindTexture(localCtx);
        drawNodes(localCtx, skybox, closest);

        //m_curr->unbind(ctx);

        ctx.bindClipPlanesUBO();

        m_debugViewport->setTextureID(m_curr->m_spec.attachments[0].textureID);
    }

    m_prev.swap(m_curr);

    ctx.bindMatricesUBO();

    m_rendered = true;
}

void MirrorMapRenderer::drawNodes(
    const RenderContext& ctx,
    SkyboxRenderer* skybox,
    Node* current)
{
    {
        int mask = GL_DEPTH_BUFFER_BIT;
        if (ctx.assets.clearColor) {
            if (ctx.assets.debugClearColor) {
                glClearColor(0.9f, 0.0f, 0.9f, 1.0f);
            }
            mask |= GL_COLOR_BUFFER_BIT;
        }
        glClear(mask);
    }

    ctx.bindClipPlanesUBO();
    //ctx.state.enable(GL_CLIP_DISTANCE0);
    {
        auto renderTypes = [&ctx, &current](const MeshTypeMap& typeMap) {
            auto shader = typeMap.begin()->first.type->m_nodeShader;

            for (const auto& it : typeMap) {
                auto& type = *it.first.type;
                auto& batch = ctx.m_batch;

                if (type.m_flags.noReflect) continue;

                for (auto& node : it.second) {
                    if (node == current) continue;
                    batch.draw(ctx, *node, shader);
                }
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
    //ctx.state.disable(GL_CLIP_DISTANCE0);

    ctx.m_batch.flush(ctx);
}

Node* MirrorMapRenderer::findClosest(const RenderContext& ctx)
{
    const glm::vec3& cameraPos = ctx.m_camera.getPos();
    const glm::vec3& cameraFront = ctx.m_camera.getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : ctx.registry.allNodes) {
        for (const auto& [key, nodes] : all.second) {
            auto& type = key.type;

            if (!type->m_flags.mirror) continue;

            for (const auto& node : nodes) {
                const auto& planeNormal = node->getWorldPlaneNormal();

                node->m_tagMaterialIndex = -1;

                const auto eyeV = node->getWorldPos() - cameraPos;
                const auto dist = glm::length(eyeV);
                auto eyeN = glm::normalize(eyeV);

                const auto dot = glm::dot(planeNormal, -eyeN);
                if (dot <= 0) {
                    // NOTE KI backside; ignore
                    continue;
                }

                sorted[glm::length(eyeV)] = node;
            }
        }
    }

    for (std::map<float, Node*>::iterator it = sorted.begin(); it != sorted.end(); ++it) {
        return (Node*)it->second;
    }
    return nullptr;
}
