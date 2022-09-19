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
    nodeRenderer = std::make_unique<NodeRenderer>(assets);
    //terrainRenderer = std::make_unique<TerrainRenderer>(assets);

    viewportRenderer = std::make_unique<ViewportRenderer>(assets);

    waterMapRenderer = std::make_unique<WaterMapRenderer>(assets);
    cubeMapRenderer = std::make_unique<CubeMapRenderer>(assets);
    shadowMapRenderer = std::make_unique<ShadowMapRenderer>(assets);

    objectIdRenderer = std::make_unique<ObjectIdRenderer>(assets);
    normalRenderer = std::make_unique<NormalRenderer>(assets);

    particleSystem = std::make_unique<ParticleSystem>(assets);
}

Scene::~Scene()
{
    KI_INFO_SB("SCENE: deleted");

    particleGenerators.clear();
}

void Scene::prepare(ShaderRegistry& shaders)
{
    prepareUBOs();

    // NOTE KI OpenGL does NOT like interleaved draw and prepare
    if (nodeRenderer) {
        nodeRenderer->prepare(shaders);
    }
    //terrainRenderer->prepare(shaders);

    if (viewportRenderer) {
        viewportRenderer->prepare(shaders);
    }

    if (waterMapRenderer) {
        waterMapRenderer->prepare(shaders);
    }
    if (cubeMapRenderer) {
        cubeMapRenderer->prepare(shaders);
    }
    if (shadowMapRenderer) {
        shadowMapRenderer->prepare(shaders);
    }

    if (objectIdRenderer) {
        objectIdRenderer->prepare(shaders);
    }

    if (assets.showNormals) {
        if (normalRenderer) {
            normalRenderer->prepare(shaders);
        }
    }

    if (particleSystem) {
        particleSystem->prepare(shaders);
    }

    {
        mainViewport = std::make_shared<Viewport>(
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            -1,
            shaders.getShader(assets, TEX_VIEWPORT));

        //mainViewport->effect = ViewportEffect::edge;

        mainViewport->prepare();
        registry.addViewPort(mainViewport);
    }

    if (objectIdRenderer) {
        registry.addViewPort(objectIdRenderer->debugViewport);
    }

    if (!mirrorBuffer && assets.showMirrorView) {
        auto buffer = new TextureBuffer({ 640, 480, { FrameBufferAttachment::getTexture(), FrameBufferAttachment::getRBODepthStencil() } });
        mirrorBuffer.reset(buffer);
        mirrorBuffer->prepare();

        mirrorViewport = std::make_shared<Viewport>(
            glm::vec3(0.5, 1, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            mirrorBuffer->spec.attachments[0].textureID,
            shaders.getShader(assets, TEX_VIEWPORT));

        mirrorViewport->prepare();
        registry.addViewPort(mirrorViewport);
    }

    if (assets.showShadowMapView) {
        registry.addViewPort(shadowMapRenderer->debugViewport);
    }
    if (assets.showReflectionView) {
        registry.addViewPort(waterMapRenderer->reflectionDebugViewport);
    }
    if (assets.showRefractionView) {
        registry.addViewPort(waterMapRenderer->refractionDebugViewport);
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
    if (dirLight) {
        dirLight->update(ctx);
    }
    for (auto light : pointLights) {
        light->update(ctx);
    }
    for (auto light : spotLights) {
        light->update(ctx);
    }

    for (auto generator : particleGenerators) {
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

    {
        if (shadowMapRenderer) {
            shadowMapRenderer->render(ctx, registry);
            shadowMapRenderer->bindTexture(ctx);
        }

        if (cubeMapRenderer) {
            cubeMapRenderer->render(ctx, registry, skyboxRenderer.get());
        }
        if (waterMapRenderer) {
            waterMapRenderer->render(ctx, registry, skyboxRenderer.get());
        }
    }

    {
        drawMain(ctx);
        drawMirror(ctx);
        drawViewports(ctx);
    }
}

void Scene::drawMain(RenderContext& ctx)
{
    RenderContext mainCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, ctx.camera, mainBuffer->spec.width, mainBuffer->spec.height);
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

    RenderContext mirrorCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, camera, mirrorBuffer->spec.width, mirrorBuffer->spec.height);
    mirrorCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;
    mirrorCtx.bindMatricesUBO();

    mirrorBuffer->bind(mirrorCtx);

    drawScene(mirrorCtx);

    mirrorBuffer->unbind(ctx);
    ctx.bindMatricesUBO();
}

void Scene::drawViewports(RenderContext& ctx)
{
    ctx.state.disable(GL_DEPTH_TEST);
    ctx.state.enable(GL_BLEND);
    if (viewportRenderer) {
        viewportRenderer->render(ctx, registry);
    }
    ctx.state.disable(GL_BLEND);
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

    //ctx.state.enable(GL_CLIP_DISTANCE0);
    //ClipPlaneUBO& clip = ctx.clipPlanes.clipping[0];
    //clip.enabled = true;
    //clip.plane = glm::vec4(0, -1, 0, 15);
    //ctx.bindClipPlanesUBO();

    {
        // NOTE KI multitarget *WAS* just to support ObjectID, which is now separate renderer
        // => If shader needs it need to define some logic
        int bufferCount = 1;

        GLenum buffers[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
        };
        if (bufferCount > 1) {
            glDrawBuffers(bufferCount, buffers);
            {
                // NOTE KI this was *ONLY* for ObjectID case
                //glm::vec4 bg{ 0.f, 0.f, 0.f, 1.f };
                //glClearBufferfv(GL_COLOR, 1, glm::value_ptr(bg));
            }
        }

        if (nodeRenderer) {
            nodeRenderer->renderSelectionStencil(ctx, registry);
            nodeRenderer->render(ctx, registry);
        }

        if (skyboxRenderer) {
            skyboxRenderer->render(ctx, registry);
        }

        if (nodeRenderer) {
            nodeRenderer->renderBlended(ctx, registry);
            nodeRenderer->renderSelection(ctx, registry);
        }

        if (bufferCount > 1) {
            glDrawBuffers(1, buffers);
        }
    }

    //clip.enabled = false;
    //ctx.bindClipPlanesUBO();
    //ctx.state.disable(GL_CLIP_DISTANCE0);

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
    return cameraNode ? cameraNode->camera.get() : nullptr;
}

Node* Scene::getCameraNode()
{
    return cameraNode;
}

Light* Scene::getDirLight()
{
    return dirLight;
}

std::vector<Light*>& Scene::getPointLights()
{
    return pointLights;
}

std::vector<Light*>& Scene::getSpotLights()
{
    return spotLights;
}

void Scene::bindComponents(Node* node)
{
    if (node->camera) {
        cameraNode = node;
    }

    Light* light = node->light.get();
    if (node->light) {
        if (light->directional) {
            dirLight = light;
        }
        else if (light->point) {
            pointLights.push_back(light);
        }
        else if (light->spot) {
            spotLights.push_back(light);
        }
    }

    if (node->particleGenerator) {
        if (particleSystem) {
            node->particleGenerator->system = particleSystem.get();
            particleGenerators.push_back(node->particleGenerator.get());
        }
    }
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
    if (!mainBuffer) {
        auto buffer = new TextureBuffer({ ctx.width, ctx.height, { FrameBufferAttachment::getTexture(), FrameBufferAttachment::getRBODepthStencil() } });
        mainBuffer.reset(buffer);
        mainBuffer->prepare();
        mainViewport->setTextureID(mainBuffer->spec.attachments[0].textureID);
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
