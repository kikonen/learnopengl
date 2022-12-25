#include "Scene.h"

#include <thread>
#include <mutex>

#include "ki/GL.h"
#include "ki/Timer.h"

#include "asset/UBO.h"


Scene::Scene(const Assets& assets)
    : assets(assets),
    m_registry(assets),
    m_materialRegistry(assets),
    m_typeRegistry(assets),
    m_modelRegistry(assets)
{
    NodeListener listener = [this](Node* node, NodeOperation operation) {
        bindComponents(*node);
    };
    m_registry.addListener(listener);

    m_nodeRenderer = std::make_unique<NodeRenderer>();
    //terrainRenderer = std::make_unique<TerrainRenderer>();

    m_viewportRenderer = std::make_unique<ViewportRenderer>();

    m_waterMapRenderer = std::make_unique<WaterMapRenderer>();
    m_mirrorMapRenderer = std::make_unique<MirrorMapRenderer>();
    //m_cubeMapRenderer = std::make_unique<CubeMapRenderer>();
    m_shadowMapRenderer = std::make_unique<ShadowMapRenderer>();

    m_objectIdRenderer = std::make_unique<ObjectIdRenderer>();
    m_normalRenderer = std::make_unique<NormalRenderer>();

    particleSystem = std::make_unique<ParticleSystem>();
}

Scene::~Scene()
{
    KI_INFO_SB("SCENE: deleted");

    particleGenerators.clear();
}

void Scene::prepare(ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    prepareUBOs();

    m_commandEngine.prepare(assets);
    m_scriptEngine.prepare(assets, m_commandEngine);

    m_batch.prepare(assets, assets.batchSize);

    m_materialRegistry.prepare();

    m_modelRegistry.prepare(m_batch);

    m_registry.prepare(&m_batch, &m_materialRegistry, &m_modelRegistry);

    // NOTE KI OpenGL does NOT like interleaved draw and prepare
    if (m_nodeRenderer) {
        m_nodeRenderer->prepare(assets, shaders);
    }
    //terrainRenderer->prepare(shaders);

    if (m_viewportRenderer) {
        m_viewportRenderer->prepare(assets, shaders);
    }

    if (m_waterMapRenderer) {
        m_waterMapRenderer->prepare(assets, shaders);
    }
    if (m_mirrorMapRenderer) {
        m_mirrorMapRenderer->prepare(assets, shaders);
    }
    if (m_cubeMapRenderer) {
        m_cubeMapRenderer->prepare(assets, shaders);
    }
    if (m_shadowMapRenderer) {
        m_shadowMapRenderer->prepare(assets, shaders);
    }

    if (m_objectIdRenderer) {
        m_objectIdRenderer->prepare(assets, shaders);
    }

    if (assets.showNormals) {
        if (m_normalRenderer) {
            m_normalRenderer->prepare(assets, shaders);
        }
    }

    if (particleSystem) {
        particleSystem->prepare(assets, shaders);
    }

    {
        m_mainViewport = std::make_shared<Viewport>(
            "Main",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            0,
            shaders.getShader(assets, TEX_VIEWPORT));

        m_mainViewport->m_effect = assets.viewportEffect;

        m_mainViewport->prepare(assets);
        m_registry.addViewPort(m_mainViewport);
    }

    if (assets.showObjectIDView) {
        if (m_objectIdRenderer) {
            m_registry.addViewPort(m_objectIdRenderer->m_debugViewport);
        }
    }

    if (assets.showRearView) {
        m_rearViewport = std::make_shared<Viewport>(
            "Rear",
            glm::vec3(0.5, 1, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            0,
            shaders.getShader(assets, TEX_VIEWPORT));

        m_rearViewport->prepare(assets);
        m_registry.addViewPort(m_rearViewport);
    }

    if (assets.showShadowMapView) {
        if (m_shadowMapRenderer) {
            m_registry.addViewPort(m_shadowMapRenderer->m_debugViewport);
        }
    }
    if (assets.showReflectionView) {
        if (m_waterMapRenderer) {
            m_registry.addViewPort(m_waterMapRenderer->m_reflectionDebugViewport);
        }
        if (m_mirrorMapRenderer) {
            m_registry.addViewPort(m_mirrorMapRenderer->m_debugViewport);
        }
    }
    if (assets.showRefractionView) {
        if (m_waterMapRenderer) {
            m_registry.addViewPort(m_waterMapRenderer->m_refractionDebugViewport);
        }
    }
}

void Scene::attachNodes()
{
    m_registry.attachNodes();
}

void Scene::processEvents(RenderContext& ctx)
{
}

void Scene::update(RenderContext& ctx)
{
    //if (ctx.clock.frameCount > 120) {
    if (getCamera()) {
        m_commandEngine.update(ctx);
    }

    if (m_registry.m_root) {
        m_registry.m_root->update(ctx, nullptr);
    }

    for (auto& generator : particleGenerators) {
        generator->update(ctx);
    }

    if (m_skyboxRenderer) {
        m_skyboxRenderer->update(ctx);
    }

    if (m_nodeRenderer) {
        m_nodeRenderer->update(ctx);
    }

    if (m_objectIdRenderer) {
        m_objectIdRenderer->update(ctx);
    }

    if (m_viewportRenderer) {
        m_viewportRenderer->update(ctx);
    }

    if (particleSystem) {
        particleSystem->update(ctx);
    }

    m_materialRegistry.update(ctx);

    updateMainViewport(ctx);
}

void Scene::bind(RenderContext& ctx)
{
    //if (nodeRenderer) {
    //    nodeRenderer->bind(ctx);
    //}
    //terrainRenderer->bind(ctx);

    //if (viewportRenderer) {
    //    viewportRenderer->bind(ctx);
    //}
    //if (waterMapRenderer) {
    //    waterMapRenderer->bind(ctx);
    //}
    //if (mirrorMapRenderer) {
    //    mirrorMapRenderer->bind(ctx);
    //}
    //if (cubeMapRenderer) {
    //    cubeMapRenderer->bind(ctx);
    //}
    if (m_shadowMapRenderer) {
        m_shadowMapRenderer->bind(ctx);
    }

    ctx.bindDefaults();
    ctx.bindUBOs();
}


void Scene::unbind(RenderContext& ctx)
{
}

void Scene::draw(RenderContext& ctx)
{
    // NOTE KI this clears *window* buffer, not actual "main" buffer used for drawing
    // => Stencil is not supposed to exist here
    if (assets.clearColor) {
        if (assets.debugClearColor) {
            //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClearColor(0.9f, 0.9f, 0.0f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else {
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    glDepthFunc(GL_LEQUAL);

    //glUseProgram(0);

    if (m_shadowMapRenderer) {
        m_shadowMapRenderer->render(ctx);
        m_shadowMapRenderer->bindTexture(ctx);
    }

    // OpenGL Programming Guide, 8th Edition, page 404
    // Enable polygon offset to resolve depth-fighting isuses
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    if (m_cubeMapRenderer) {
        m_cubeMapRenderer->render(ctx, m_skyboxRenderer.get());
    }
    if (m_waterMapRenderer) {
        m_waterMapRenderer->render(ctx, m_skyboxRenderer.get());
    }
    if (m_mirrorMapRenderer) {
        m_mirrorMapRenderer->render(ctx, m_skyboxRenderer.get());
    }

    {
        drawMain(ctx);
        drawRear(ctx);
        drawViewports(ctx);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void Scene::drawMain(RenderContext& ctx)
{
    RenderContext mainCtx("MAIN", &ctx, ctx.m_camera, m_mainBuffer->m_spec.width, m_mainBuffer->m_spec.height);
    mainCtx.m_matrices.lightProjected = ctx.m_matrices.lightProjected;

    m_mainBuffer->bind(mainCtx);
    drawScene(mainCtx);
    m_mainBuffer->unbind(ctx);
}

// "back mirror" viewport
void Scene::drawRear(RenderContext& ctx)
{
    if (!assets.showRearView) return;

    Camera camera(ctx.m_camera.getPos(), ctx.m_camera.getFront(), ctx.m_camera.getUp());
    camera.setZoom(ctx.m_camera.getZoom());

    glm::vec3 rot = ctx.m_camera.getRotation();
    //rot.y += 180;
    rot.y += 180;
    camera.setRotation(-rot);

    RenderContext mirrorCtx("BACK", &ctx, camera, m_readBuffer->m_spec.width, m_readBuffer->m_spec.height);
    mirrorCtx.m_matrices.lightProjected = ctx.m_matrices.lightProjected;
    mirrorCtx.bindMatricesUBO();

    m_readBuffer->bind(mirrorCtx);

    drawScene(mirrorCtx);

    m_readBuffer->unbind(ctx);
    ctx.bindMatricesUBO();
}

void Scene::drawViewports(RenderContext& ctx)
{
    if (m_viewportRenderer) {
        m_viewportRenderer->render(ctx);
    }
}

void Scene::drawScene(RenderContext& ctx)
{
    // NOTE KI clear for current draw buffer buffer (main/mirror/etc.)
    if (assets.clearColor) {
        if (assets.debugClearColor) {
            glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else {
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    m_materialRegistry.bind(ctx);

    if (m_cubeMapRenderer) {
        m_cubeMapRenderer->bindTexture(ctx);
    }
    if (m_waterMapRenderer) {
        m_waterMapRenderer->bindTexture(ctx);
    }
    if (m_mirrorMapRenderer) {
        m_mirrorMapRenderer->bindTexture(ctx);
    }

    if (m_nodeRenderer) {
        m_nodeRenderer->render(ctx, m_skyboxRenderer.get());
    }

    if (particleSystem) {
        particleSystem->render(ctx);
    }

    if (assets.showNormals) {
        if (m_normalRenderer) {
            m_normalRenderer->render(ctx);
        }
    }
}

Camera* Scene::getCamera()
{
    return !m_registry.m_cameraNodes.empty() ? m_registry.m_cameraNodes[0]->m_camera.get() : nullptr;
}

void Scene::bindComponents(Node& node)
{
    auto& type = node.m_type;

    if (node.m_particleGenerator) {
        if (particleSystem) {
            node.m_particleGenerator->system = particleSystem.get();
            particleGenerators.push_back(node.m_particleGenerator.get());
        }
    }

    m_scriptEngine.registerScript(node, NodeScriptId::init, type->m_initScript);
    m_scriptEngine.registerScript(node, NodeScriptId::run, type->m_runScript);

    m_scriptEngine.runScript(node, NodeScriptId::init);
}

int Scene::getObjectID(const RenderContext& ctx, double screenPosX, double screenPosY)
{
    if (m_objectIdRenderer) {
        m_objectIdRenderer->render(ctx);
        return m_objectIdRenderer->getObjectId(ctx, screenPosX, screenPosY, m_mainViewport.get());
    }
    return 0;
}

void Scene::updateMainViewport(RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;
    int w = ctx.assets.resolutionScale.x * res.x;
    int h = ctx.assets.resolutionScale.y * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !m_mainBuffer || w != m_mainBuffer->m_spec.width || h != m_mainBuffer->m_spec.height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO_SB("BUFFER: create - w=" << w << ", h=" << h);

    // MAIN
    {
        // NOTE KI alpha NOT needed
        auto buffer = new TextureBuffer({
            w, h,
            { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepthStencil() } });

        m_mainBuffer.reset(buffer);
        m_mainBuffer->prepare(true, { 0, 0, 0, 1.0 });
        m_mainViewport->setTextureID(m_mainBuffer->m_spec.attachments[0].textureID);
    }

    // VMIRROR
    {
        int mirrorW = w * 0.5;
        int mirrorH = h * 0.5;

        if (mirrorW < 1) mirrorW = 1;
        if (mirrorH < 1) mirrorH = 1;

        if (!m_readBuffer && assets.showRearView) {
            // NOTE KI alpha NOT needed
            auto buffer = new TextureBuffer({
                mirrorW, mirrorH,
                { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepthStencil() } });

            m_readBuffer.reset(buffer);
            m_readBuffer->prepare(true, { 0, 0, 0, 1.0 });

            m_rearViewport->setTextureID(m_readBuffer->m_spec.attachments[0].textureID);
        }
    }
}

void Scene::prepareUBOs()
{
    // Matrices
    {
        int sz = sizeof(MatricesUBO);

        m_ubo.matrices.create();
        m_ubo.matrices.initEmpty(sz, GL_DYNAMIC_STORAGE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATRICES, m_ubo.matrices, 0, sz);
        m_ubo.matricesSize = sz;
    }
    // Data
    {
        int sz = sizeof(DataUBO);

        m_ubo.data.create();
        m_ubo.data.initEmpty(sz, GL_DYNAMIC_STORAGE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_DATA, m_ubo.data, 0, sz);
        m_ubo.dataSize = sz;
    }
    // Clipping
    {
        int sz = sizeof(ClipPlanesUBO);

        m_ubo.clipPlanes.create();
        m_ubo.clipPlanes.initEmpty(sz, GL_DYNAMIC_STORAGE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_CLIP_PLANES, m_ubo.clipPlanes, 0, sz);
        m_ubo.clipPlanesSize = sz;
    }
    // Lights
    {
        int sz = sizeof(LightsUBO);

        m_ubo.lights.create();
        m_ubo.lights.initEmpty(sz, GL_DYNAMIC_STORAGE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_LIGHTS, m_ubo.lights, 0, sz);
        m_ubo.lightsSize = sz;
    }
    // Textures
    {
        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
        // https://www.khronos.org/opengl/wiki/Bindless_Texture

        // https://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/        //glGenBuffers(1, &ssbo);
        //glGenBuffers(1, &m_ubo.textures);
        //glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ubo.textures);
        //glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TexturesUBO), &m_textures, GL_DYNAMIC_COPY);
        //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        int sz = sizeof(TexturesUBO);

        m_ubo.textures.create();
        m_ubo.textures.initEmpty(sz, GL_MAP_WRITE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_TEXTURES, m_ubo.textures, 0, sz);
        m_ubo.texturesSize = sz;

        m_textureHandles = (TextureUBO*)glMapNamedBufferRange(
            m_ubo.textures,
            0,
            sz,
            GL_MAP_WRITE_BIT |
            GL_MAP_FLUSH_EXPLICIT_BIT |
            GL_MAP_INVALIDATE_RANGE_BIT);
    }
}
