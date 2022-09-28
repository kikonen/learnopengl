#include "CubeMapRenderer.h"

#include <vector>

#include "asset/ShaderBind.h"
#include "SkyboxRenderer.h"

namespace {
    // +X (right)
    // -X (left)
    // +Y (top)
    // -Y (bottom)
    // +Z (front) 
    // -Z (back)
    const glm::vec3 CAMERA_FRONT[6] = {
        {  1,  0,  0 },
        { -1,  0,  0 },
        {  0,  1,  0 },
        {  0, -1,  0 },
        {  0,  0,  1 },
        {  0,  0, -1 },
    };

    const glm::vec3 CAMERA_UP[6] = {
        {  0, -1,  0 },
        {  0, -1,  0 },
        {  0,  0,  1 },
        {  0,  0, -1 },
        {  0, -1,  0 },
        {  0, -1,  0 },
    };

    const glm::vec4 DEBUG_COLOR[6] = {
        {  1,  0,  0, 1 },
        {  0,  1,  0, 1 },
        {  0,  0,  1, 1 },
        {  1,  1,  0, 1 },
        {  0,  1,  1, 1 },
        {  1,  0,  1, 1 },
    };
}


CubeMapRenderer::CubeMapRenderer()
{
}

CubeMapRenderer::~CubeMapRenderer()
{
}

void CubeMapRenderer::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    drawIndex = 0;
    drawSkip = assets.cubeMapDrawSkip;

    Renderer::prepare(assets, shaders);

    curr = std::make_unique<DynamicCubeMap>(assets.cubeMapSize);
    curr->prepare(false, { 0, 0, 1, 1.0 });

    prev = std::make_unique<DynamicCubeMap>(assets.cubeMapSize);
    prev->prepare(false, { 0, 1, 0, 1.0 });
}

void CubeMapRenderer::bind(const RenderContext& ctx)
{
}

void CubeMapRenderer::bindTexture(const RenderContext& ctx)
{
    //if (!rendered) return;
    prev->bindTexture(ctx, ctx.assets.cubeMapUnitIndex);
}

void CubeMapRenderer::render(
    const RenderContext& mainCtx,
    const NodeRegistry& registry,
    SkyboxRenderer* skybox)
{
    if (!cleared) {
        clearCubeMap(mainCtx, *prev.get(), { 0, 0, 0, 1 }, false);
        clearCubeMap(mainCtx, *curr.get(), { 0, 0, 0, 1 }, false);
        cleared = true;
    }

    if (!stepRender()) return;

    Node* centerNode = findCenter(mainCtx, registry);
    if (!centerNode) return;

    // https://www.youtube.com/watch?v=lW_iqrtJORc
    // https://eng.libretexts.org/Bookshelves/Computer_Science/Book%3A_Introduction_to_Computer_Graphics_(Eck)/07%3A_3D_Graphics_with_WebGL/7.04%3A_Framebuffers
    // view-source:math.hws.edu/eck/cs424/graphicsbook2018/source/webgl/cube-camera.html

    curr->bind(mainCtx);

    const glm::vec3& center = centerNode->getPos();

    for (int i = 0; i < 6; i++) {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, curr->textureID, 0);

        if (mainCtx.assets.clearColor) {
            if (mainCtx.assets.debugClearColor) {
                auto color = DEBUG_COLOR[i];
                glClearColor(color.r, color.g, color.b, color.a);
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        else {
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        Camera camera(center, CAMERA_FRONT[i], CAMERA_UP[i]);
        camera.setZoom(90.0);
        RenderContext ctx("CUBE", mainCtx.assets, mainCtx.clock, mainCtx.state, mainCtx.scene, camera, curr->size, curr->size);
        bindTexture(ctx);
        ctx.lightSpaceMatrix = mainCtx.lightSpaceMatrix;
        ctx.bindMatricesUBO();

        drawNodes(ctx, registry, skybox, centerNode);
    }

    curr->unbind(mainCtx);

    prev.swap(curr);

    rendered = true;
}

void CubeMapRenderer::clearCubeMap(
    const RenderContext& ctx,
    DynamicCubeMap& cube,
    const glm::vec4& color,
    bool debug)
{
    cube.bind(ctx);

    for (int i = 0; i < 6; i++) {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cube.textureID, 0);

        auto c = color;
        if (debug) c = DEBUG_COLOR[i];
        glClearColor(c.r, c.g, c.b, c.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    cube.unbind(ctx);
}

void CubeMapRenderer::drawNodes(
    const RenderContext& ctx,
    const NodeRegistry& registry,
    SkyboxRenderer* skybox,
    const Node* centerNode)
{
    auto renderTypes = [&ctx, &centerNode](const NodeTypeMap& typeMap) {
        ShaderBind bound(typeMap.begin()->first->nodeShader);

        for (const auto& [type, nodes] : typeMap) {
            Batch& batch = type->batch;

            //ShaderBind bound(type->defaultShader);

            type->bind(ctx, bound.shader);
            batch.bind(ctx, bound.shader);

            for (auto& node : nodes) {
                // NOTE KI skip drawing center node itself (can produce odd results)
                // => i.e. show garbage from old render round and such
                if (node == centerNode) continue;

                batch.draw(ctx, node, bound.shader);
            }

            batch.flush(ctx, type);
            type->unbind(ctx);
        }
    };

    for (const auto& all : registry.solidNodes) {
        renderTypes(all.second);
    }

    for (const auto& all : registry.alphaNodes) {
        renderTypes(all.second);
    }

    // NOTE KI skybox MUST be rendered before blended nodes
    if (skybox) {
        skybox->render(ctx);
    }

    for (const auto& all : registry.blendedNodes) {
        renderTypes(all.second);
    }
}

Node* CubeMapRenderer::findCenter(const RenderContext& ctx, const NodeRegistry& registry)
{
    const glm::vec3& cameraPos = ctx.camera.getPos();
    const glm::vec3& cameraDir = ctx.camera.getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : registry.allNodes) {
        for (const auto& [type, nodes] : all.second) {
            if (!(type->hasReflection() || type->hasRefraction())) continue;

            for (const auto& node : nodes) {
                const glm::vec3 ray = node->getPos() - cameraPos;
                const float distance = glm::length(ray);
                const glm::vec3 fromCamera = glm::normalize(ray);
                const float dot = glm::dot(fromCamera, cameraDir);
                if (dot < 0) continue;

                sorted[-distance] = node;
            }
        }
    }

    for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
        return it->second;
    }
    return nullptr;
}

