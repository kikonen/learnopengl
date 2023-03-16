#include "Scene.h"

#include <thread>
#include <mutex>

#include "ki/GL.h"
#include "ki/Timer.h"

#include "asset/UBO.h"
#include "asset/Shader.h"

#include "component/ParticleGenerator.h"

#include "controller/NodeController.h"

#include "renderer/NodeRenderer.h"
//#include "renderer/TerrainRenderer.h"
#include "renderer/ViewportRenderer.h"

#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "renderer/SkyboxRenderer.h"

#include "renderer/NodeDraw.h"

#include "registry/MaterialRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"

#include "scene/Batch.h"

#include "scene/UpdateContext.h"
#include "scene/RenderContext.h"

#include "scene/RenderData.h"

#include "command/api/ResumeNode.h"

Scene::Scene(
    const Assets& assets,
    std::shared_ptr<Registry> registry)
    : m_assets(assets),
    m_alive(std::make_shared<std::atomic<bool>>(true)),
    m_registry(registry)
{
    m_commandEngine = std::make_unique<CommandEngine>(assets);
    m_scriptEngine = std::make_unique<ScriptEngine>(assets);

    m_eventQueue = std::make_unique<event::Queue>(assets);

    NodeListener listener = [this](Node* node, NodeOperation operation) {
        bindComponents(node);
    };
    m_registry->m_nodeRegistry->addListener(listener);

    m_nodeRenderer = std::make_unique<NodeRenderer>();
    //terrainRenderer = std::make_unique<TerrainRenderer>();

    m_viewportRenderer = std::make_unique<ViewportRenderer>();

    if (m_assets.renderWaterMap) {
        m_waterMapRenderer = std::make_unique<WaterMapRenderer>();
    }
    if (m_assets.renderMirrorMap) {
        m_mirrorMapRenderer = std::make_unique<MirrorMapRenderer>();
    }
    if (m_assets.renderCubeMap) {
        m_cubeMapRenderer = std::make_unique<CubeMapRenderer>();
    }
    if (m_assets.renderShadowMap) {
        m_shadowMapRenderer = std::make_unique<ShadowMapRenderer>();
    }

    m_objectIdRenderer = std::make_unique<ObjectIdRenderer>();
    m_normalRenderer = std::make_unique<NormalRenderer>();

    m_particleSystem = std::make_unique<ParticleSystem>();

    m_batch = std::make_unique<Batch>();
    m_nodeDraw = std::make_unique<NodeDraw>();
    m_renderData = std::make_unique<RenderData>();
}

Scene::~Scene()
{
    *m_alive = false;
    m_particleGenerators.clear();

    KI_INFO("SCENE: deleted");
}

void Scene::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    m_renderData->prepare();

    m_commandEngine->prepare();
    m_scriptEngine->prepare(*m_commandEngine);

    m_batch->prepare(m_assets, m_registry.get());
    m_nodeDraw->prepare(m_assets, m_registry.get());

    // NOTE KI OpenGL does NOT like interleaved draw and prepare
    if (m_nodeRenderer) {
        m_nodeRenderer->prepare(m_assets, m_registry.get());
    }
    //terrainRenderer->prepare(programs);

    if (m_viewportRenderer) {
        m_viewportRenderer->prepare(m_assets, m_registry.get());
    }

    if (m_waterMapRenderer) {
        m_waterMapRenderer->prepare(m_assets, m_registry.get());
    }
    if (m_mirrorMapRenderer) {
        m_mirrorMapRenderer->prepare(m_assets, m_registry.get());
    }
    if (m_cubeMapRenderer) {
        m_cubeMapRenderer->prepare(m_assets, m_registry.get());
    }
    if (m_shadowMapRenderer) {
        m_shadowMapRenderer->prepare(m_assets, m_registry.get());
    }

    if (m_objectIdRenderer) {
        m_objectIdRenderer->prepare(m_assets, m_registry.get());
    }

    if (m_assets.showNormals) {
        if (m_normalRenderer) {
            m_normalRenderer->prepare(m_assets, m_registry.get());
        }
    }

    if (m_particleSystem) {
        m_particleSystem->prepare(m_assets, m_registry.get());
    }

    {
        m_windowBuffer = std::make_unique<WindowBuffer>();
    }

    {
        m_mainViewport = std::make_shared<Viewport>(
            "Main",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            false,
            0,
            m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

        m_mainViewport->setEffectEnabled(m_assets.viewportEffectEnabled);
        m_mainViewport->setEffect(m_assets.viewportEffect);
        m_mainViewport->prepare(m_assets);

        m_registry->m_nodeRegistry->addViewPort(m_mainViewport);

        m_registry->m_physicsEngine->prepare();
    }

    if (m_assets.showObjectIDView) {
        if (m_objectIdRenderer) {
            m_registry->m_nodeRegistry->addViewPort(m_objectIdRenderer->m_debugViewport);
        }
    }

    if (m_assets.showRearView) {
        m_rearViewport = std::make_shared<Viewport>(
            "Rear",
            glm::vec3(0.5, 1, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            true,
            0,
            m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

        m_rearViewport->prepare(m_assets);
        m_registry->m_nodeRegistry->addViewPort(m_rearViewport);
    }

    if (m_assets.showShadowMapView) {
        if (m_shadowMapRenderer) {
            m_registry->m_nodeRegistry->addViewPort(m_shadowMapRenderer->m_debugViewport);
        }
    }
    if (m_assets.showReflectionView) {
        if (m_waterMapRenderer) {
            m_registry->m_nodeRegistry->addViewPort(m_waterMapRenderer->m_reflectionDebugViewport);
        }
        if (m_mirrorMapRenderer) {
            m_registry->m_nodeRegistry->addViewPort(m_mirrorMapRenderer->m_debugViewport);
        }
    }
    if (m_assets.showRefractionView) {
        if (m_waterMapRenderer) {
            m_registry->m_nodeRegistry->addViewPort(m_waterMapRenderer->m_refractionDebugViewport);
        }
    }
}

void Scene::attachNodes()
{
    m_registry->m_nodeRegistry->attachNodes();
}

void Scene::processEvents(const UpdateContext& ctx)
{
    m_eventQueue->dispatchEvents(ctx);
}

void Scene::update(const UpdateContext& ctx)
{
    //if (ctx.clock.frameCount > 120) {
    if (getActiveCamera()) {
        m_commandEngine->update(ctx);
    }

    if (m_registry->m_nodeRegistry->m_root) {
        m_registry->m_nodeRegistry->m_root->update(ctx, nullptr);
        m_registry->m_physicsEngine->update(ctx);
    }

    for (auto& generator : m_particleGenerators) {
        generator->update(ctx);
    }

    if (m_viewportRenderer) {
        m_viewportRenderer->update(ctx);
    }

    if (m_particleSystem) {
        m_particleSystem->update(ctx);
    }

    m_registry->m_materialRegistry->update(ctx);
    m_registry->m_entityRegistry->update(ctx);

    m_renderData->update();
}

void Scene::updateView(const RenderContext& ctx)
{
    if (m_objectIdRenderer) {
        m_objectIdRenderer->updateView(ctx);
    }

    updateMainViewport(ctx);

    m_nodeDraw->updateView(ctx);
    m_windowBuffer->updateView(ctx);
}

void Scene::bind(const RenderContext& ctx)
{
    if (m_shadowMapRenderer) {
        m_shadowMapRenderer->bind(ctx);
    }
    if (m_registry->m_nodeRegistry->m_skybox) {
        m_registry->m_nodeRegistry->m_skybox->bindTexture(ctx);
    }

    m_renderData->bind();

    ctx.m_data.u_cubeMapExist = m_cubeMapRenderer && m_cubeMapRenderer->isRendered();

    ctx.bindDefaults();
    ctx.updateUBOs();

    m_batch->bind();
}


void Scene::unbind(const RenderContext& ctx)
{
}

backend::gl::PerformanceCounters Scene::getCounters(bool clear)
{
    return m_batch->getCounters(clear);
}

backend::gl::PerformanceCounters Scene::getCountersLocal(bool clear)
{
    return m_batch->getCountersLocal(clear);
}

void Scene::draw(const RenderContext& ctx)
{
    ctx.state.setDepthFunc(ctx.m_depthFunc);

    bool wasCubeMap = false;
    int renderCount = 0;

    if (m_shadowMapRenderer && m_shadowMapRenderer->render(ctx)) {
        renderCount++;
        m_shadowMapRenderer->bindTexture(ctx);
    }

    // OpenGL Programming Guide, 8th Edition, page 404
    // Enable polygon offset to resolve depth-fighting isuses
    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(0.2f, 0.2f);
    ctx.state.enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    if (m_cubeMapRenderer && m_cubeMapRenderer->render(ctx)) {
        //wasCubeMap = true;
    }

    if (!wasCubeMap && m_waterMapRenderer && m_waterMapRenderer->render(ctx))
        renderCount++;

    if (!wasCubeMap && m_mirrorMapRenderer && m_mirrorMapRenderer->render(ctx))
        renderCount++;

    // NOTE KI skip main render if special update cycle
    //if (!wasCubeMap) // && renderCount <= 2)
    {
        drawMain(ctx);
        drawRear(ctx);
    }
    drawViewports(ctx);

    //glDisable(GL_POLYGON_OFFSET_FILL);
}

void Scene::drawMain(const RenderContext& ctx)
{
    RenderContext mainCtx("MAIN", &ctx, ctx.m_camera, m_mainBuffer->m_spec.width, m_mainBuffer->m_spec.height);

    mainCtx.m_matrices.u_lightProjected = ctx.m_matrices.u_lightProjected;
    mainCtx.m_matrices.u_shadow = ctx.m_matrices.u_shadow;

    drawScene(mainCtx, m_mainBuffer.get());
}

// "back mirror" viewport
void Scene::drawRear(const RenderContext& parentCtx)
{
    if (!m_assets.showRearView) return;

    auto* mainCamera = parentCtx.m_camera;

    Camera camera(mainCamera->getWorldPosition(), mainCamera->getFront(), mainCamera->getUp());
    camera.setZoom(mainCamera->getZoom());

    glm::vec3 rot = mainCamera->getRotation();
    rot.y += 180;
    camera.setRotation(-rot);

    RenderContext localCtx("BACK", &parentCtx, &camera, m_rearBuffer->m_spec.width, m_rearBuffer->m_spec.height);

    localCtx.m_matrices.u_lightProjected = parentCtx.m_matrices.u_lightProjected;
    localCtx.m_matrices.u_shadow = parentCtx.m_matrices.u_shadow;

    localCtx.updateMatricesUBO();
    localCtx.updateDataUBO();

    drawScene(localCtx, m_rearBuffer.get());

    parentCtx.updateMatricesUBO();
    parentCtx.updateDataUBO();
}

void Scene::drawViewports(const RenderContext& ctx)
{
    m_windowBuffer->bind(ctx);

    // NOTE KI this clears *window* buffer, not actual "main" buffer used for drawing
    // => Stencil is not supposed to exist here
    // => no need to clear this; ViewPort will do glBlitNamedFramebuffer
    // => *BUT* if glDraw is used instead then clear *IS* needed for depth
    if (false) {
        int mask = GL_DEPTH_BUFFER_BIT;
        if (m_assets.clearColor) {
            if (m_assets.debugClearColor) {
                //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClearColor(0.9f, 0.9f, 0.0f, 0.0f);
            }
            mask |= GL_COLOR_BUFFER_BIT;
        }
        glClear(mask);
    }

    if (m_viewportRenderer) {
        m_viewportRenderer->render(ctx, m_windowBuffer.get());
    }
}

void Scene::drawScene(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    m_registry->m_materialRegistry->bind(ctx);
    m_registry->m_entityRegistry->bind(ctx);

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
        m_nodeRenderer->render(ctx, targetBuffer);
    }

    targetBuffer->bind(ctx);

    if (m_particleSystem) {
        m_particleSystem->render(ctx);
    }

    if (m_assets.showNormals) {
        if (m_normalRenderer) {
            m_normalRenderer->render(ctx);
        }
    }
}

Node* Scene::getActiveCamera() const
{
    return m_registry->m_nodeRegistry->getActiveCamera();
}

NodeController* Scene::getActiveCameraController() const
{
    auto* node = getActiveCamera();
    return node ? node->m_controller.get() : nullptr;
}

void Scene::bindComponents(Node* node)
{
    auto& type = node->m_type;

    if (node->m_particleGenerator) {
        if (m_particleSystem) {
            node->m_particleGenerator->system = m_particleSystem.get();
            m_particleGenerators.push_back(node->m_particleGenerator.get());
        }
    }

    if (m_assets.useScript) {
        m_scriptEngine->registerNode(node);
        m_scriptEngine->registerScript(node, NodeScriptId::init, type->m_script);

        m_scriptEngine->runScript(node, NodeScriptId::init);

        // NOTE KI start via queue, in sync with next update cycle
        if (m_scriptEngine->hasFunction(node, "start")) {
            m_commandEngine->addCommand(
                std::make_unique<ResumeNode>(
                    -1,
                    node->m_objectID,
                    "start"));
        }
    }
}

int Scene::getObjectID(const RenderContext& ctx, double screenPosX, double screenPosY)
{
    if (m_objectIdRenderer) {
        m_objectIdRenderer->render(ctx);
        return m_objectIdRenderer->getObjectId(ctx, screenPosX, screenPosY, m_mainViewport.get());
    }
    return 0;
}

void Scene::updateMainViewport(const RenderContext& ctx)
{
    const auto& res = ctx.m_resolution;

    int w = ctx.m_assets.resolutionScale * res.x;
    int h = ctx.m_assets.resolutionScale * res.y;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    bool changed = !m_mainBuffer || w != m_mainBuffer->m_spec.width || h != m_mainBuffer->m_spec.height;
    if (!changed) return;

    //if (m_mainBuffer) return;
    KI_INFO(fmt::format("FRAME_BUFFER: update - w={}, h={}", w, h));

    // MAIN
    {
        // NOTE KI alpha NOT needed
        auto buffer = new TextureBuffer({
            w, h,
            { FrameBufferAttachment::getTextureRGBA(), FrameBufferAttachment::getRBODepthStencil() } });

        m_mainBuffer.reset(buffer);
        m_mainBuffer->prepare(true, { 0, 0, 0, 0.0 });

        m_mainViewport->setTextureId(m_mainBuffer->m_spec.attachments[0].textureID);
        m_mainViewport->setSourceFrameBuffer(m_mainBuffer.get());
    }

    // VMIRROR
    {
        int mirrorW = w * 0.5;
        int mirrorH = h * 0.5;

        if (mirrorW < 1) mirrorW = 1;
        if (mirrorH < 1) mirrorH = 1;

        if (!m_rearBuffer && m_assets.showRearView) {
            // NOTE KI alpha NOT needed
            auto buffer = new TextureBuffer({
                mirrorW, mirrorH,
                { FrameBufferAttachment::getTextureRGBA(), FrameBufferAttachment::getRBODepthStencil() } });

            m_rearBuffer.reset(buffer);
            m_rearBuffer->prepare(true, { 0, 0, 0, 0.0 });

            m_rearViewport->setTextureId(m_rearBuffer->m_spec.attachments[0].textureID);
            m_rearViewport->setSourceFrameBuffer(m_rearBuffer.get());
        }
    }
}
