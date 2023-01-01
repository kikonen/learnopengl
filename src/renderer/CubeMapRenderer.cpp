#include "CubeMapRenderer.h"

#include <vector>

#include "SkyboxRenderer.h"

#include "registry/MaterialRegistry.h"

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

void CubeMapRenderer::prepare(
    const Assets& assets,
    ShaderRegistry& shaders,
    MaterialRegistry& materialRegistry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders, materialRegistry);

    m_tagID = assets.cubeMapUUID;
    m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
    m_tagMaterial.kd = glm::vec4(0.f, 0.8f, 0.8f, 1.f);
    materialRegistry.add(m_tagMaterial);

    m_renderFrequency = assets.cubeMapRenderFrequency;

    m_nearPlane = assets.cubeMapNearPlane;
    m_farPlane = assets.cubeMapFarPlane;

    m_curr = std::make_unique<DynamicCubeMap>(assets.cubeMapSize);
    m_curr->prepare(false, { 0, 0, 1, 1.0 });

    m_prev = std::make_unique<DynamicCubeMap>(assets.cubeMapSize);
    m_prev->prepare(false, { 0, 1, 0, 1.0 });

    glm::vec3 origo(0);
    for (int i = 0; i < 6; i++) {
        auto& camera = m_cameras.emplace_back(origo, CAMERA_FRONT[i], CAMERA_UP[i]);
        camera.setZoom(90.0);
    }
}

void CubeMapRenderer::bindTexture(const RenderContext& ctx)
{
    //if (!rendered) return;
    m_prev->bindTexture(ctx, UNIT_CUBE_MAP);
}

void CubeMapRenderer::render(
    const RenderContext& mainCtx,
    SkyboxRenderer* skybox)
{
    if (!m_cleared) {
        clearCubeMap(mainCtx, *m_prev.get(), { 0, 0, 0, 1 }, false);
        clearCubeMap(mainCtx, *m_curr.get(), { 0, 0, 0, 1 }, false);
        m_cleared = true;
    }

    if (!needRender(mainCtx)) return;

    Node* centerNode = findCenter(mainCtx);
    if (!centerNode) return;

    if (mainCtx.assets.showCubeMapCenter) {
        Node* tagNode = getTagNode(mainCtx.registry);
        if (tagNode) {
            const auto& rootPos = mainCtx.registry.m_root->getPosition();
            const auto& centerPos = centerNode->getWorldPos();
            const auto tagPos = centerPos - rootPos;
            tagNode->setPosition(tagPos);
            tagNode->m_type->m_flags.noDisplay = false;
            //tagNode->m_tagMaterialIndex = m_tagMaterial.m_registeredIndex;
        }
    }

    // https://www.youtube.com/watch?v=lW_iqrtJORc
    // https://eng.libretexts.org/Bookshelves/Computer_Science/Book%3A_Introduction_to_Computer_Graphics_(Eck)/07%3A_3D_Graphics_with_WebGL/7.04%3A_Framebuffers
    // view-source:math.hws.edu/eck/cs424/graphicsbook2018/source/webgl/cube-camera.html

    m_curr->bind(mainCtx);

    for (int i = 0; i < 6; i++) {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            m_curr->m_textureID,
            0);

        {
            int mask = GL_DEPTH_BUFFER_BIT;
            if (mainCtx.assets.clearColor) {
                if (mainCtx.assets.debugClearColor) {
                    auto color = DEBUG_COLOR[i];
                    glClearColor(color.r, color.g, color.b, color.a);
                }
                mask |= GL_COLOR_BUFFER_BIT;
            }
            glClear(mask);
        }

        // centerNode->getVolume()->getRadius();

        const auto& center = centerNode->getWorldPos();
        auto& camera = m_cameras[i];
        camera.setPos(center);

        RenderContext ctx("CUBE",
            &mainCtx, camera,
            m_nearPlane,
            m_farPlane,
            m_curr->m_size, m_curr->m_size);
        bindTexture(ctx);
        ctx.m_matrices.lightProjected = mainCtx.m_matrices.lightProjected;
        ctx.bindMatricesUBO();

        drawNodes(ctx, skybox, centerNode);
    }

    m_curr->unbind(mainCtx);

    m_prev.swap(m_curr);

    m_rendered = true;
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
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cube.m_textureID, 0);

        auto c = color;
        if (debug) c = DEBUG_COLOR[i];
        glClearColor(c.r, c.g, c.b, c.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    cube.unbind(ctx);
}

void CubeMapRenderer::drawNodes(
    const RenderContext& ctx,
    SkyboxRenderer* skybox,
    const Node* centerNode)
{
    auto renderTypes = [&ctx, &centerNode](const MeshTypeMap& typeMap) {
        auto shader = typeMap.begin()->first.type->m_nodeShader;

        for (const auto& it : typeMap) {
            auto& type = *it.first.type;
            auto& batch = ctx.m_batch;

            if (type.m_flags.noReflect) continue;

            for (auto& node : it.second) {
                // NOTE KI skip drawing center node itself (can produce odd results)
                // => i.e. show garbage from old render round and such
                if (node == centerNode) continue;

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

    // NOTE KI skybox MUST be rendered before blended nodes
    if (skybox) {
        skybox->render(ctx);
    }

    for (const auto& all : ctx.registry.blendedNodes) {
        renderTypes(all.second);
    }

    ctx.m_batch.flush(ctx);
}

Node* CubeMapRenderer::findCenter(const RenderContext& ctx)
{
    const glm::vec3& cameraPos = ctx.m_camera.getPos();
    const glm::vec3& cameraDir = ctx.m_camera.getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : ctx.registry.allNodes) {
        for (const auto& [key, nodes] : all.second) {
            auto& type = key.type;

            if (!type->m_flags.cubeMap) continue;

            for (const auto& node : nodes) {
                const glm::vec3 ray = node->getWorldPos() - cameraPos;
                const float distance = std::abs(glm::length(ray));

                if (false) {
                    const glm::vec3 fromCamera = glm::normalize(ray);
                    const float dot = glm::dot(fromCamera, cameraDir);
                    if (dot < 0) continue;
                }

                sorted[distance] = node;
            }
        }
    }

    for (std::map<float, Node*>::iterator it = sorted.begin(); it != sorted.end(); ++it) {
        return it->second;
    }
    return nullptr;
}

Node* CubeMapRenderer::getTagNode(NodeRegistry& registry)
{
    if (m_tagNode) return m_tagNode;
    m_tagNode = registry.getNode(m_tagID);
    return m_tagNode;
}
