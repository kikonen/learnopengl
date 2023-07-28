#include "CubeMapRenderer.h"

#include <vector>

#include "asset/Shader.h"
#include "asset/DynamicCubeMap.h"
#include "render/RenderContext.h"
#include "render/Batch.h"
#include "render/FrameBuffer.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"

#include "render/NodeDraw.h"
#include "render/CubeMapBuffer.h"

#include "registry/MeshType.h"


// https://stackoverflow.com/questions/28845375/rendering-a-dynamic-cubemap-opengl
// https://mbroecker.com/project_dynamic_cubemapping.html

namespace {
    //    basePath + "/right.jpg",
    //    basePath + "/left.jpg",
    //    basePath + "/top.jpg",
    //    basePath + "/bottom.jpg",
    //    basePath + "/front.jpg",
    //    basePath + "/back.jpg"

    //#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
    //#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
    //#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
    //#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
    //#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
    //#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A

    // https://dsweeneyblog.wordpress.com/2016/10/03/dynamic-cube-mapping-in-opengl/
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
        {  1,  0,  0, 0 },
        {  0,  1,  0, 0 },
        {  0,  0,  1, 0 },
        {  1,  1,  0, 0 },
        {  0,  1,  1, 0 },
        {  1,  0,  1, 0 },
    };
}

CubeMapRenderer::~CubeMapRenderer()
{}

void CubeMapRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, registry);

    m_renderFrameStart = assets.cubeMapRenderFrameStart;
    m_renderFrameStep = assets.cubeMapRenderFrameStep;

    m_tagID = assets.cubeMapUUID;
    m_tagMaterial = Material::createMaterial(BasicMaterial::highlight);
    m_tagMaterial.kd = glm::vec4(0.f, 0.8f, 0.8f, 1.f);
    m_registry->m_materialRegistry->add(m_tagMaterial);

    m_nearPlane = assets.cubeMapNearPlane;
    m_farPlane = assets.cubeMapFarPlane;

    int size = assets.cubeMapSize;
    int scaledSize = assets.bufferScale * size;

    {
        m_curr = std::make_unique<DynamicCubeMap>(scaledSize);
        m_curr->prepare(
            false, { 0, 0, 1.f, 1.f });

        m_prev = std::make_unique<DynamicCubeMap>(scaledSize);
        m_prev->prepare(
            false,
            { 0, 1.f, 0, 1.f });
    }

    glm::vec3 origo{ 0 };
    for (int face = 0; face < 6; face++) {
        auto& camera = m_cameras.emplace_back(origo, CAMERA_FRONT[face], CAMERA_UP[face]);
        camera.setFov(90.f);
    }
}

void CubeMapRenderer::bindTexture(const RenderContext& ctx)
{
    //if (!rendered) return;
    m_prev->bindTexture(ctx, UNIT_CUBE_MAP);
}

bool CubeMapRenderer::render(
    const RenderContext& parentCtx)
{
    parentCtx.validateRender("cube_map");

    if (!m_cleared) {
        clearCubeMap(parentCtx, *m_prev.get());
        clearCubeMap(parentCtx, *m_curr.get());
        m_cleared = true;
    }

    if (!needRender(parentCtx)) return false;

    Node* centerNode = findCenter(parentCtx);
    if (m_lastClosest && setClosest(centerNode, -1)) {
        //std::cout << "NEW_CENTER\n";
        m_curr->m_updateFace = -1;
        m_prev->m_updateFace = -1;
        m_curr->m_rendered = false;
        m_prev->m_rendered = false;
    }
    if (!centerNode) return false;

    if (parentCtx.m_assets.showCubeMapCenter) {
        Node* tagNode = getTagNode();
        if (tagNode) {
            const auto& rootPos = parentCtx.m_registry->m_nodeRegistry->m_root->getPosition();
            const auto& centerPos = centerNode->getWorldPosition();
            const auto tagPos = centerPos - rootPos;
            tagNode->setPosition(tagPos);
            tagNode->m_type->m_flags.noDisplay = false;
            //tagNode->m_tagMaterialIndex = m_tagMaterial.m_registeredIndex;
        }
    }

    // https://www.youtube.com/watch?v=lW_iqrtJORc
    // https://eng.libretexts.org/Bookshelves/Computer_Science/Book%3A_Introduction_to_Computer_Graphics_(Eck)/07%3A_3D_Graphics_with_WebGL/7.04%3A_Framebuffers
    // view-source:math.hws.edu/eck/cs424/graphicsbook2018/source/webgl/cube-camera.html

    //m_curr->bind(mainCtx);

    unsigned int fromFace = m_curr->m_updateFace;
    unsigned int updateCount = 1;
    bool full = true;// fromFace == -1;

    if (full) {
        //std::cout << "FULL\n";
        fromFace = 0;
        updateCount = 6;
    }
    else {
        //std::cout << "update: " << m_updateFace << "\n";
    }

    clearCubeMap(parentCtx, *m_curr.get());

    for (unsigned int face = fromFace; face < fromFace + updateCount; face++) {
        //glFramebufferTexture2D(
        //    GL_FRAMEBUFFER,
        //    GL_COLOR_ATTACHMENT0,
        //    GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
        //    m_curr->m_cubeMap.m_textureID,
        //    0);

        glm::vec4 debugColor{ DEBUG_COLOR[face] };

        // centerNode->getVolume()->getRadius();

        const auto& center = centerNode->getWorldPosition();
        auto& camera = m_cameras[face];
        camera.setWorldPosition(center);

        RenderContext localCtx("CUBE",
            &parentCtx,
            &camera,
            m_nearPlane,
            m_farPlane,
            m_curr->m_size, m_curr->m_size);

        bindTexture(localCtx);

        localCtx.copyShadowFrom(parentCtx);

        localCtx.updateMatricesUBO();
        localCtx.updateDataUBO();

        auto targetBuffer = m_curr->asFrameBuffer(face);
        //targetBuffer.clear(ctx, clearColor);
        drawNodes(localCtx, &targetBuffer, centerNode, debugColor);
    }

    if (full)
        m_curr->m_rendered = true;

    //m_curr->unbind(mainCtx);
    m_prev.swap(m_curr);

    parentCtx.updateMatricesUBO();
    parentCtx.updateDataUBO();

    if (m_curr->m_rendered) {
        m_curr->m_updateFace = (m_curr->m_updateFace + 1) % 6;
    }

    m_rendered = true;
    return true;
}

void CubeMapRenderer::clearCubeMap(
    const RenderContext& ctx,
    DynamicCubeMap& cube)
{
    cube.bind(ctx);

    glm::vec4 clearColor{ 0.f };

    for (int face = 0; face < 6; face++) {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            cube.m_cubeMap.m_textureID,
            0);

        ctx.m_state.clearColor(clearColor);

        glClear(GL_COLOR_BUFFER_BIT);
    }

    cube.unbind(ctx);
}

void CubeMapRenderer::drawNodes(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer,
    const Node* current,
    const glm::vec4& debugColor)
{
    // TODO KI to match special logic in CubeMapBuffer
    targetBuffer->bind(ctx);

    ctx.m_nodeDraw->drawNodes(
        ctx,
        targetBuffer,
        [](const MeshType* type) { return !type->m_flags.noReflect; },
        // NOTE KI skip drawing center node itself (can produce odd results)
        // => i.e. show garbage from old render round and such
        [&current](const Node* node) { return node != current; },
        GL_COLOR_BUFFER_BIT);
}

Node* CubeMapRenderer::findCenter(const RenderContext& ctx)
{
    const glm::vec3& cameraPos = ctx.m_camera->getWorldPosition();
    const glm::vec3& cameraDir = ctx.m_camera->getViewFront();

    std::map<float, Node*> sorted;

    for (const auto& all : ctx.m_registry->m_nodeRegistry->allNodes) {
        for (const auto& [key, nodes] : all.second) {
            auto& type = key.type;

            if (!type->m_flags.cubeMap) continue;

            for (const auto& node : nodes) {
                const glm::vec3 ray = node->getWorldPosition() - cameraPos;
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

Node* CubeMapRenderer::getTagNode()
{
    if (m_tagNode) return m_tagNode;
    m_tagNode = m_registry->m_nodeRegistry->getNode(m_tagID);
    return m_tagNode;
}
