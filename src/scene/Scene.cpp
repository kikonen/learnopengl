#include "Scene.h"

#include <iostream>

#include "kigl/GLState.h"

#include "util/thread.h"

#include "asset/Assets.h"
#include "asset/Shader.h"
#include "asset/DynamicCubeMap.h"

#include "asset/LightUBO.h"

#include "model/Viewport.h"

#include "event/Dispatcher.h"

#include "controller/NodeController.h"

#include "registry/Registry.h"
#include "registry/MaterialRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/ProgramRegistry.h"
#include "registry/MeshTypeRegistry.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"
#include "engine/UpdateViewContext.h"

#include "render/DebugContext.h"
#include "render/NodeDraw.h"
#include "render/Batch.h"
#include "render/RenderContext.h"
#include "render/WindowBuffer.h"
#include "render/RenderData.h"

#include "renderer/NodeRenderer.h"
#include "renderer/ViewportRenderer.h"

#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/NormalRenderer.h"

namespace {
}

Scene::Scene(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_alive(alive),
    m_registry(registry)
{
    const auto& assets = Assets::get();

    {
        m_mainRenderer = std::make_unique<NodeRenderer>("main", true);
        m_rearRenderer = std::make_unique<NodeRenderer>("rear", true);

        m_viewportRenderer = std::make_unique<ViewportRenderer>(true);

        m_waterMapRenderer = std::make_unique<WaterMapRenderer>("main", true, true, false);
        m_mirrorMapRenderer = std::make_unique<MirrorMapRenderer>("main", true, true, false);
        m_cubeMapRenderer = std::make_unique<CubeMapRenderer>(true);
        m_shadowMapRenderer = std::make_unique<ShadowMapRenderer>(true);

        m_objectIdRenderer = std::make_unique<ObjectIdRenderer>(false);
        m_normalRenderer = std::make_unique<NormalRenderer>(false);

        m_mainRenderer->setEnabled(true);
        m_rearRenderer->setEnabled(true);

        m_viewportRenderer->setEnabled(true);

        m_waterMapRenderer->setEnabled(assets.waterMapEnabled);
        m_mirrorMapRenderer->setEnabled(assets.mirrorMapEnabled);
        m_cubeMapRenderer->setEnabled(assets.cubeMapEnabled);
        m_shadowMapRenderer->setEnabled(assets.shadowMapEnabled);

        m_objectIdRenderer->setEnabled(true);
        m_normalRenderer->setEnabled(assets.showNormals);
    }

    m_batch = std::make_unique<render::Batch>();
    m_nodeDraw = std::make_unique<render::NodeDraw>();
    m_renderData = std::make_unique<render::RenderData>();
}

Scene::~Scene()
{
    *m_alive = false;

    KI_INFO("SCENE: deleted");
}

void Scene::destroy()
{
    KI_INFO("SCENE: destroy");
}

void Scene::prepareRT()
{
    const auto& assets = Assets::get();

    std::cout << "RT: worker=" << util::isWorkerThread() << '\n';

    auto* dispatcherView = m_registry->m_dispatcherView;

    dispatcherView->addListener(
        event::Type::scene_loaded,
        [this](const event::Event& e) {
            m_loaded = true;
        });

    dispatcherView->addListener(
        event::Type::node_added,
        [this](const event::Event& e) {
            auto* node = pool::NodeHandle::toNode(e.body.node.target);
            this->handleNodeAdded(node);
        });

    m_renderData->prepare(
        false,
        assets.glUseInvalidate,
        assets.glUseFence,
        assets.glUseSingleFence,
        assets.glUseDebugFence,
        assets.batchDebug);

    PrepareContext ctx{ m_registry.get() };

    m_batch->prepareRT(ctx);
    m_nodeDraw->prepareRT(ctx);

    m_mainRenderer->prepareRT(ctx);

    // NOTE KI OpenGL does NOT like interleaved draw and prepare
    if (m_rearRenderer->isEnabled()) {
        m_rearRenderer->prepareRT(ctx);
    }
    if (m_rearRenderer->isEnabled()) {
        m_rearRenderer->prepareRT(ctx);
    }

    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->prepareRT(ctx);
    }

    if (m_waterMapRenderer->isEnabled()) {
        m_waterMapRenderer->prepareRT(ctx);
    }
    if (m_mirrorMapRenderer->isEnabled()) {
        m_mirrorMapRenderer->prepareRT(ctx);
    }
    if (m_cubeMapRenderer->isEnabled()) {
        m_cubeMapRenderer->prepareRT(ctx);
    }
    if (m_shadowMapRenderer->isEnabled()) {
        m_shadowMapRenderer->prepareRT(ctx);
    }

    if (m_objectIdRenderer->isEnabled()) {
        m_objectIdRenderer->prepareRT(ctx);
    }

    if (assets.showNormals) {
        m_normalRenderer->prepareRT(ctx);
    }

    {
        m_windowBuffer = std::make_unique<render::WindowBuffer>(true);
    }

    {
        m_mainViewport = std::make_shared<Viewport>(
            "Node",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        m_mainViewport->setUpdate([](Viewport& vp, const UpdateViewContext& ctx) {
        });

        m_mainViewport->setBindBefore([this](Viewport& vp) {
            auto* buffer = m_mainRenderer->m_buffer.get();
            vp.setTextureId(buffer->m_spec.attachments[NodeRenderer::ATT_ALBEDO_INDEX].textureID);
            vp.setSourceFrameBuffer(buffer);
        });

        m_mainViewport->setGammaCorrect(true);
        m_mainViewport->setHardwareGamma(true);

        m_mainViewport->setEffectEnabled(assets.viewportEffectEnabled);
        m_mainViewport->setEffect(assets.viewportEffect);

        m_mainViewport->prepareRT();
        ViewportRegistry::get().addViewport(m_mainViewport);
    }

    if (assets.showRearView) {
        m_rearViewport = std::make_shared<Viewport>(
            "Rear",
            glm::vec3(-1.f, -0.5f, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            true,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        m_rearViewport->setBindBefore([this](Viewport& vp) {
            auto* buffer = m_rearRenderer->m_buffer.get();
            vp.setTextureId(buffer->m_spec.attachments[NodeRenderer::ATT_ALBEDO_INDEX].textureID);
            vp.setSourceFrameBuffer(buffer);
        });

        m_rearViewport->setGammaCorrect(true);
        m_rearViewport->setHardwareGamma(true);

        m_rearViewport->prepareRT();
        ViewportRegistry::get().addViewport(m_rearViewport);
    }

    if (assets.showObjectIDView) {
        if (m_objectIdRenderer->isEnabled()) {
            ViewportRegistry::get().addViewport(m_objectIdRenderer->m_debugViewport);
        }
    }

    if (assets.showShadowMapView) {
        if (m_shadowMapRenderer->isEnabled()) {
            ViewportRegistry::get().addViewport(m_shadowMapRenderer->m_debugViewport);
        }
    }
    if (assets.showReflectionView) {
        if (m_waterMapRenderer->isEnabled()) {
            ViewportRegistry::get().addViewport(m_waterMapRenderer->m_reflectionDebugViewport);
        }
        if (m_mirrorMapRenderer->isEnabled()) {
            ViewportRegistry::get().addViewport(m_mirrorMapRenderer->m_reflectionDebugViewport);
        }
    }
    if (assets.showRefractionView) {
        if (m_waterMapRenderer->isEnabled()) {
            ViewportRegistry::get().addViewport(m_waterMapRenderer->m_refractionDebugViewport);
        }
    }
}

void Scene::updateRT(const UpdateContext& ctx)
{
    const auto& assets = ctx.m_assets;
    const auto& debugContext = ctx.m_debugContext;

    m_registry->m_dispatcherView->dispatchEvents();

    m_registry->updateRT(ctx);

    m_renderData->update();

    m_normalRenderer->setEnabled(assets.showNormals && debugContext.m_nodeDebugEnabled && debugContext.m_showNormals);

    m_batch->updateRT(ctx);
}

void Scene::postRT(const UpdateContext& ctx)
{
    m_registry->postRT(ctx);
}

void Scene::updateViewRT(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;

    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->updateRT(ctx);
    }

    if (m_objectIdRenderer->isEnabled()) {
        m_objectIdRenderer->updateRT(ctx);
    }

    m_mainRenderer->updateRT(ctx);

    if (assets.showRearView) {
        m_rearRenderer->updateRT(ctx);
    }

    m_cubeMapRenderer->updateRT(ctx);
    m_mirrorMapRenderer->updateRT(ctx);
    m_waterMapRenderer->updateRT(ctx);

    m_nodeDraw->updateRT(ctx);
    m_windowBuffer->updateRT(ctx);
}

void Scene::handleNodeAdded(Node* node)
{
    if (!node) return;

    NodeRegistry::get().handleNodeAdded(node);
    m_nodeDraw->handleNodeAdded(node);
    m_mirrorMapRenderer->handleNodeAdded(node);
    m_waterMapRenderer->handleNodeAdded(node);
    m_cubeMapRenderer->handleNodeAdded(node);
}

void Scene::bind(const RenderContext& ctx)
{
    if (m_shadowMapRenderer->isEnabled()) {
        m_shadowMapRenderer->bind(ctx);
    }

    MeshTypeRegistry::get().bind(ctx);

    m_renderData->bind();

    ctx.m_data.u_cubeMapExist = m_cubeMapRenderer->isEnabled() && m_cubeMapRenderer->isRendered();

    ctx.bindDefaults();
    ctx.updateUBOs();

    m_batch->bind();
}


void Scene::unbind(const RenderContext& ctx)
{
}

backend::gl::PerformanceCounters Scene::getCounters(bool clear) const
{
    return m_batch->getCounters(clear);
}

backend::gl::PerformanceCounters Scene::getCountersLocal(bool clear) const
{
    return m_batch->getCountersLocal(clear);
}

void Scene::draw(const RenderContext& ctx)
{
    const auto& assets = ctx.m_assets;
    auto& state = ctx.m_state;

    state.setDepthFunc(ctx.m_depthFunc);

    bool wasCubeMap = false;
    int renderCount = 0;

    if (m_shadowMapRenderer->render(ctx)) {
        renderCount++;
        m_shadowMapRenderer->bindTexture(ctx);
    }

    state.setEnabled(GL_TEXTURE_CUBE_MAP_SEAMLESS, assets.cubeMapSeamless);

    if (m_cubeMapRenderer->render(ctx)) {
        wasCubeMap = assets.cubeMapSkipOthers;
    }

    if (!wasCubeMap && m_waterMapRenderer->render(ctx))
        renderCount++;

    if (!wasCubeMap && m_mirrorMapRenderer->render(ctx))
        renderCount++;

    // NOTE KI skip main render if special update cycle
    //if (!wasCubeMap) // && renderCount <= 2)
    {
        drawMain(ctx);
        drawRear(ctx);
    }
    drawViewports(ctx);
}

void Scene::drawMain(const RenderContext& parentCtx)
{
    RenderContext localCtx(
        "MAIN",
        &parentCtx,
        parentCtx.m_camera,
        m_mainRenderer->m_buffer->m_spec.width,
        m_mainRenderer->m_buffer->m_spec.height);

    localCtx.copyShadowFrom(parentCtx);

    localCtx.m_allowDrawDebug = true;
    drawScene(localCtx, m_mainRenderer.get());
}

// "back mirror" viewport
void Scene::drawRear(const RenderContext& parentCtx)
{
    const auto& assets = parentCtx.m_assets;

    if (!assets.showRearView) return;

    auto* parentCamera = parentCtx.m_camera;

    glm::vec3 cameraFront = parentCamera->getViewFront() * -1.f;

    Camera camera(
        parentCamera->getWorldPosition(),
        parentCamera->getFront(),
        parentCamera->getUp());
    camera.setViewport(parentCamera->getViewport());
    camera.setFov(parentCamera->getFov());
    camera.setAxis(cameraFront, parentCamera->getViewUp());

    RenderContext localCtx(
        "BACK",
        &parentCtx,
        &camera,
        m_rearRenderer->m_buffer->m_spec.width,
        m_rearRenderer->m_buffer->m_spec.height);

    localCtx.copyShadowFrom(parentCtx);

    localCtx.updateMatricesUBO();
    localCtx.updateDataUBO();

    drawScene(localCtx, m_rearRenderer.get());

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
    //
    // NOTE KI *CLEAR* buffer
    // - https://stackoverflow.com/questions/37335281/is-glcleargl-color-buffer-bit-preferred-before-a-whole-frame-buffer-overwritte
    //
    m_windowBuffer->clear(
        ctx,
        GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
        { 0.9f, 0.9f, 0.0f, 0.0f });

    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->render(ctx, m_windowBuffer.get());
    }
}

void Scene::drawScene(
    const RenderContext& ctx,
    NodeRenderer* nodeRenderer)
{
    const auto& assets = ctx.m_assets;

    //MaterialRegistry::get().bind(ctx);
    EntityRegistry::get().bind(ctx);

    if (m_cubeMapRenderer->isEnabled()) {
        m_cubeMapRenderer->bindTexture(ctx);
    }
    if (m_waterMapRenderer->isEnabled()) {
        m_waterMapRenderer->bindTexture(ctx);
    }
    if (m_mirrorMapRenderer->isEnabled()) {
        m_mirrorMapRenderer->bindTexture(ctx);
    }

    if (nodeRenderer->isEnabled()) {
        nodeRenderer->render(ctx, nodeRenderer->m_buffer.get());
    }

    if (m_normalRenderer->isEnabled()) {
        m_normalRenderer->render(ctx);
    }
}

Node* Scene::getActiveNode() const
{
    return NodeRegistry::get().getActiveNode();
}

const std::vector<NodeController*>* Scene::getActiveNodeControllers() const
{
    auto* node = getActiveNode();
    return node ? ControllerRegistry::get().forNode(node) : nullptr;
}

Node* Scene::getActiveCameraNode() const
{
    return NodeRegistry::get().getActiveCameraNode();
}

const std::vector<NodeController*>* Scene::getActiveCameraControllers() const
{
    auto* node = getActiveCameraNode();
    return node ? ControllerRegistry::get().forNode(node) : nullptr;
}

ki::node_id Scene::getObjectID(const RenderContext& ctx, float screenPosX, float screenPosY)
{
    if (m_objectIdRenderer->isEnabled()) {
        m_objectIdRenderer->render(ctx);
        return m_objectIdRenderer->getObjectId(ctx, screenPosX, screenPosY, m_mainViewport.get());
    }
    return 0;
}
