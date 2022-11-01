#include "Scene.h"

#include <thread>
#include <mutex>

#include "ki/GL.h"
#include "ki/Timer.h"

#include "asset/UBO.h"


Scene::Scene(const Assets& assets)
    : assets(assets),
    registry(*this)
{
    nodeRenderer = std::make_unique<NodeRenderer>();
    //terrainRenderer = std::make_unique<TerrainRenderer>();

    viewportRenderer = std::make_unique<ViewportRenderer>();

    waterMapRenderer = std::make_unique<WaterMapRenderer>();
    mirrorMapRenderer = std::make_unique<MirrorMapRenderer>();
    cubeMapRenderer = std::make_unique<CubeMapRenderer>();
    shadowMapRenderer = std::make_unique<ShadowMapRenderer>();

    objectIdRenderer = std::make_unique<ObjectIdRenderer>();
    normalRenderer = std::make_unique<NormalRenderer>();

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

    commandEngine.prepare(assets);
    scriptEngine.prepare(assets, commandEngine);

    // NOTE KI OpenGL does NOT like interleaved draw and prepare
    if (nodeRenderer) {
        nodeRenderer->prepare(assets, shaders);
    }
    //terrainRenderer->prepare(shaders);

    if (viewportRenderer) {
        viewportRenderer->prepare(assets, shaders);
    }

    if (waterMapRenderer) {
        waterMapRenderer->prepare(assets, shaders);
    }
    if (mirrorMapRenderer) {
        mirrorMapRenderer->prepare(assets, shaders);
    }
    if (cubeMapRenderer) {
        cubeMapRenderer->prepare(assets, shaders);
    }
    if (shadowMapRenderer) {
        shadowMapRenderer->prepare(assets, shaders);
    }

    if (objectIdRenderer) {
        objectIdRenderer->prepare(assets, shaders);
    }

    if (assets.showNormals) {
        if (normalRenderer) {
            normalRenderer->prepare(assets, shaders);
        }
    }

    if (particleSystem) {
        particleSystem->prepare(assets, shaders);
    }

    {
        mainViewport = std::make_shared<Viewport>(
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            0,
            shaders.getShader(assets, TEX_VIEWPORT));

        //mainViewport->effect = ViewportEffect::edge;

        mainViewport->prepare(assets);
        registry.addViewPort(mainViewport);
    }

    if (assets.showObjectIDView) {
        if (objectIdRenderer) {
            registry.addViewPort(objectIdRenderer->debugViewport);
        }
    }

    if (assets.showMirrorView) {
        mirrorViewport = std::make_shared<Viewport>(
            glm::vec3(0.5, 1, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            0,
            shaders.getShader(assets, TEX_VIEWPORT));

        mirrorViewport->prepare(assets);
        registry.addViewPort(mirrorViewport);
    }

    if (assets.showShadowMapView) {
        if (shadowMapRenderer) {
            registry.addViewPort(shadowMapRenderer->debugViewport);
        }
    }
    if (assets.showReflectionView) {
        if (waterMapRenderer) {
            registry.addViewPort(waterMapRenderer->reflectionDebugViewport);
        }
        if (mirrorMapRenderer) {
            registry.addViewPort(mirrorMapRenderer->debugViewport);
        }
    }
    if (assets.showRefractionView) {
        if (waterMapRenderer) {
            registry.addViewPort(waterMapRenderer->refractionDebugViewport);
        }
    }
}

void Scene::attachNodes()
{
    registry.attachNodes();
}

void Scene::processEvents(RenderContext& ctx)
{
}

void Scene::update(RenderContext& ctx)
{
    //if (ctx.clock.frameCount > 120) {
    if (getCamera()) {
        commandEngine.update(ctx);
    }

    if (registry.m_root) {
        registry.m_root->update(ctx, nullptr);
    }

    for (auto& generator : particleGenerators) {
        generator->update(ctx);
    }

    if (skyboxRenderer) {
        skyboxRenderer->update(ctx, registry);
    }

    if (nodeRenderer) {
        nodeRenderer->update(ctx, registry);
    }

    if (objectIdRenderer) {
        objectIdRenderer->update(ctx, registry);
    }

    if (viewportRenderer) {
        viewportRenderer->update(ctx, registry);
    }

    if (particleSystem) {
        particleSystem->update(ctx);
    }

    updateMainViewport(ctx);
}

void Scene::bind(RenderContext& ctx)
{
    if (nodeRenderer) {
        nodeRenderer->bind(ctx);
    }
    //terrainRenderer->bind(ctx);

    if (viewportRenderer) {
        viewportRenderer->bind(ctx);
    }

    if (waterMapRenderer) {
        waterMapRenderer->bind(ctx);
    }
    if (mirrorMapRenderer) {
        mirrorMapRenderer->bind(ctx);
    }
    if (cubeMapRenderer) {
        cubeMapRenderer->bind(ctx);
    }
    if (shadowMapRenderer) {
        shadowMapRenderer->bind(ctx);
    }

    ctx.bindGlobal();
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

    // https://cmichel.io/understanding-front-faces-winding-order-and-normals
    ctx.state.enable(GL_CULL_FACE);
    ctx.state.cullFace(GL_BACK);
    ctx.state.frontFace(GL_CCW);

    ctx.state.enable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(0);

    if (shadowMapRenderer) {
        shadowMapRenderer->render(ctx, registry);
        shadowMapRenderer->bindTexture(ctx);
    }

    // OpenGL Programming Guide, 8th Edition, page 404
    // Enable polygon offset to resolve depth-fighting isuses
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    if (cubeMapRenderer) {
        cubeMapRenderer->render(ctx, registry, skyboxRenderer.get());
    }
    if (waterMapRenderer) {
        waterMapRenderer->render(ctx, registry, skyboxRenderer.get());
    }
    if (mirrorMapRenderer) {
        mirrorMapRenderer->render(ctx, registry, skyboxRenderer.get());
    }

    {
        drawMain(ctx);
        drawMirror(ctx);
        drawViewports(ctx);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void Scene::drawMain(RenderContext& ctx)
{
    RenderContext mainCtx("MAIN", &ctx, ctx.camera, mainBuffer->spec.width, mainBuffer->spec.height);
    mainCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

    mainBuffer->bind(mainCtx);
    drawScene(mainCtx);
    mainBuffer->unbind(ctx);
}

// "back mirror" viewport
void Scene::drawMirror(RenderContext& ctx)
{
    if (!assets.showMirrorView) return;

    Camera camera(ctx.camera.getPos(), ctx.camera.getFront(), ctx.camera.getUp());
    camera.setZoom(ctx.camera.getZoom());

    glm::vec3 rot = ctx.camera.getRotation();
    //rot.y += 180;
    rot.y += 180;
    camera.setRotation(-rot);

    RenderContext mirrorCtx("BACK", &ctx, camera, mirrorBuffer->spec.width, mirrorBuffer->spec.height);
    mirrorCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;
    mirrorCtx.bindMatricesUBO();

    mirrorBuffer->bind(mirrorCtx);

    drawScene(mirrorCtx);

    mirrorBuffer->unbind(ctx);
    ctx.bindMatricesUBO();
}

void Scene::drawViewports(RenderContext& ctx)
{
    if (viewportRenderer) {
        viewportRenderer->render(ctx, registry);
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

    if (cubeMapRenderer) {
        cubeMapRenderer->bindTexture(ctx);
    }
    if (waterMapRenderer) {
        waterMapRenderer->bindTexture(ctx);
    }
    if (mirrorMapRenderer) {
        mirrorMapRenderer->bindTexture(ctx);
    }

    if (nodeRenderer) {
        nodeRenderer->render(ctx, registry, skyboxRenderer.get());
    }

    if (particleSystem) {
        particleSystem->render(ctx);
    }

    if (assets.showNormals) {
        if (normalRenderer) {
            normalRenderer->render(ctx, registry);
        }
    }
}

Camera* Scene::getCamera()
{
    return !registry.m_cameraNodes.empty() ? registry.m_cameraNodes[0]->m_camera.get() : nullptr;
}

void Scene::bindComponents(Node& node)
{
    if (node.m_particleGenerator) {
        if (particleSystem) {
            node.m_particleGenerator->system = particleSystem.get();
            particleGenerators.push_back(node.m_particleGenerator.get());
        }
    }

    scriptEngine.registerScript(node, NodeScriptId::init, node.m_type->m_initScript);
    scriptEngine.registerScript(node, NodeScriptId::run, node.m_type->m_runScript);

    scriptEngine.runScript(node, NodeScriptId::init);
}

int Scene::getObjectID(const RenderContext& ctx, double screenPosX, double screenPosY)
{
    if (objectIdRenderer) {
        objectIdRenderer->render(ctx, registry);
        return objectIdRenderer->getObjectId(ctx, screenPosX, screenPosY, mainViewport.get());
    }
    return 0;
}

void Scene::updateMainViewport(RenderContext& ctx)
{
    const auto& res = ctx.resolution;
    int w = ctx.assets.resolutionScale.x * res.x;
    int h = ctx.assets.resolutionScale.y * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !mainBuffer || w != mainBuffer->spec.width || h != mainBuffer->spec.height;
    if (!changed) return;

    KI_INFO_SB("BUFFER: create - w=" << w << ", h=" << h);

    // MAIN
    {
        // NOTE KI alpha NOT needed
        auto buffer = new TextureBuffer({
            w, h,
            { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepthStencil() } });

        mainBuffer.reset(buffer);
        mainBuffer->prepare(true, { 0, 0, 0, 1.0 });
        mainViewport->setTextureID(mainBuffer->spec.attachments[0].textureID);
    }

    // VMIRROR
    {
        int mirrorW = w * 0.5;
        int mirrorH = h * 0.5;

        if (mirrorW < 1) mirrorW = 1;
        if (mirrorH < 1) mirrorH = 1;

        if (!mirrorBuffer && assets.showMirrorView) {
            // NOTE KI alpha NOT needed
            auto buffer = new TextureBuffer({
                mirrorW, mirrorH,
                { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepthStencil() } });

            mirrorBuffer.reset(buffer);
            mirrorBuffer->prepare(true, { 0, 0, 0, 1.0 });

            mirrorViewport->setTextureID(mirrorBuffer->spec.attachments[0].textureID);
        }
    }
}

void Scene::prepareUBOs()
{
    // Matrices
    {
        int sz = sizeof(MatricesUBO);

        glCreateBuffers(1, &ubo.matrices);
        glNamedBufferStorage(ubo.matrices, sz, nullptr, GL_DYNAMIC_STORAGE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATRICES, ubo.matrices, 0, sz);
        ubo.matricesSize = sz;
    }
    // Data
    {
        int sz = sizeof(DataUBO);

        glCreateBuffers(1, &ubo.data);
        glNamedBufferStorage(ubo.data, sz, nullptr, GL_DYNAMIC_STORAGE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_DATA, ubo.data, 0, sz);
        ubo.dataSize = sz;
    }
    // Clipping
    {
        int sz = sizeof(ClipPlanesUBO);

        glCreateBuffers(1, &ubo.clipPlanes);
        glNamedBufferStorage(ubo.clipPlanes, sz, nullptr, GL_DYNAMIC_STORAGE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_CLIP_PLANES, ubo.clipPlanes, 0, sz);
        ubo.clipPlanesSize = sz;
    }
    // Lights
    {
        int sz = sizeof(LightsUBO);

        glCreateBuffers(1, &ubo.lights);
        glNamedBufferStorage(ubo.lights, sz, nullptr, GL_DYNAMIC_STORAGE_BIT);

        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_LIGHTS, ubo.lights, 0, sz);
        ubo.lightsSize = sz;
    }
}
