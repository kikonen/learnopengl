#include "Scene.h"

#include <iostream>

#include "kigl/GLState.h"

#include "util/thread.h"

#include "asset/Assets.h"
#include "asset/DynamicCubeMap.h"

#include "shader/Shader.h"
#include "shader/LightsUBO.h"

#include "model/Viewport.h"

#include "event/Listen.h"
#include "event/Dispatcher.h"

#include "controller/NodeController.h"

#include "shader/ProgramRegistry.h"

#include "material/MaterialRegistry.h"

#include "registry/Registry.h"
#include "registry/SelectionRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/NodeTypeRegistry.h"

#include "engine/Engine.h"
#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"
#include "engine/UpdateViewContext.h"

#include "render/Camera.h"
#include "debug/DebugContext.h"
#include "render/Batch.h"
#include "render/RenderContext.h"
#include "render/WindowBuffer.h"
#include "render/RenderData.h"
#include "render/NodeDraw.h"
#include "render/NodeCollection.h"

#include "renderer/LayerRenderer.h"
#include "renderer/ViewportRenderer.h"

#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "renderer/ObjectIdRenderer.h"

namespace {
}

Scene::Scene(
    Engine& engine)
    : m_alive{ std::make_shared<std::atomic_bool>(true) },
    m_engine{ engine },
    m_collection{ std::make_unique<render::NodeCollection>()}
{
    const auto& assets = Assets::get();

    {
        m_uiRenderer = std::make_unique<LayerRenderer>("ui", true, false);
        m_playerRenderer = std::make_unique<LayerRenderer>("player", true, false);
        m_mainRenderer = std::make_unique<LayerRenderer>("main", true, true);
        m_rearRenderer = std::make_unique<LayerRenderer>("rear", true, false);

        m_viewportRenderer = std::make_unique<ViewportRenderer>(true);

        m_waterMapRenderer = std::make_unique<WaterMapRenderer>("water", true, true, false);
        m_mirrorMapRenderer = std::make_unique<MirrorMapRenderer>("mirror", true, true, false);
        m_cubeMapRenderer = std::make_unique<CubeMapRenderer>(true);
        m_shadowMapRenderer = std::make_unique<ShadowMapRenderer>(true);

        m_objectIdRenderer = std::make_unique<ObjectIdRenderer>(false);

        m_uiRenderer->setEnabled(true);
        m_playerRenderer->setEnabled(true);
        m_mainRenderer->setEnabled(true);
        m_rearRenderer->setEnabled(true);

        m_viewportRenderer->setEnabled(true);

        m_waterMapRenderer->setEnabled(assets.waterMapEnabled);
        m_mirrorMapRenderer->setEnabled(assets.mirrorMapEnabled);
        m_cubeMapRenderer->setEnabled(assets.cubeMapEnabled);
        m_shadowMapRenderer->setEnabled(assets.shadowMapEnabled);

        m_objectIdRenderer->setEnabled(true);
    }
}

Scene::~Scene()
{
    *m_alive = false;

    KI_INFO("SCENE: deleted");
}

void Scene::clear()
{
    m_collection->clear();
}

void Scene::destroy()
{
    KI_INFO("SCENE: destroy");
    clear();

    if (m_uiViewport)
    {
        ViewportRegistry::get().removeViewport(m_uiViewport->m_id);
        m_uiViewport.reset();
    }

    if (m_playerViewport)
    {
        ViewportRegistry::get().removeViewport(m_playerViewport->m_id);
        m_playerViewport.reset();
    }

    if (m_mainViewport)
    {
        ViewportRegistry::get().removeViewport(m_mainViewport->m_id);
        m_mainViewport.reset();
    }

    if (m_rearViewport)
    {
        ViewportRegistry::get().removeViewport(m_rearViewport->m_id);
        m_rearViewport.reset();
    }
}

void Scene::prepareRT()
{
    const auto& assets = Assets::get();
    const auto& dbg = debug::DebugContext::get();

    std::cout << "RT: worker=" << util::isWorkerThread() << '\n';

    auto* dispatcherView = m_engine.getRegistry()->m_dispatcherView;

    m_listen_scene_loaded.listen(
        event::Type::scene_loaded,
        dispatcherView,
        [this](const event::Event& e) {
            handleLoaded();
        });

    m_listen_node_added.listen(
        event::Type::node_added,
        dispatcherView,
        [this](const event::Event& e) {
            auto* node = pool::NodeHandle::toNode(e.body.node.target);
            this->handleNodeAdded(node);
        });

    m_listen_node_removed.listen(
        event::Type::node_removed,
        dispatcherView,
        [this](const event::Event& e) {
            auto* node = pool::NodeHandle::toNode(e.body.node.target);
            this->handleNodeRemoved(node);

            // NOTE KI tell WT that RT has diposed node
            event::Event evt{ event::Type::node_dispose };
            auto& body = evt.body.node = {
                .target = e.body.node.target,
            };
            m_engine.getRegistry()->m_dispatcherWorker->send(evt);
        });

    m_listen_camera_activate.listen(
        event::Type::camera_activate,
        dispatcherView,
        [this](const event::Event& e) {
            auto& data = e.body.node;
            auto nodeHandle = pool::NodeHandle::toHandle(data.target);
            m_collection->setActiveCameraNode(nodeHandle);
        });

    m_listen_camera_activate_next.listen(
        event::Type::camera_activate_next,
        dispatcherView,
        [this](const event::Event& e) {
            auto& data = e.body.node;
            auto nodeHandle = pool::NodeHandle::toHandle(data.target);
            auto nextCamera = m_collection->getNextCameraNode(nodeHandle, data.offset);
            m_collection->setActiveCameraNode(nextCamera);
        });

    PrepareContext ctx{ m_engine };

    //m_nodeDraw->prepareRT(ctx);

    m_uiRenderer->prepareRT(ctx);
    m_playerRenderer->prepareRT(ctx);
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

    {
        auto vp = std::make_shared<model::Viewport>(
            "UI",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        vp->setUpdate([](model::Viewport& vp, const UpdateViewContext& ctx) {
            });

        vp->setBindBefore([this](model::Viewport& vp) {
            auto* buffer = m_uiRenderer->m_buffer.get();
            vp.setTexture(
                buffer->m_spec.attachments[LayerRenderer::ATT_ALBEDO_INDEX].textureID,
                buffer->m_spec.getSize());
            vp.setSourceFrameBuffer(buffer);
            });

        if (const auto* layer = LayerInfo::findLayer(LAYER_UI); layer) {
            vp->applyLayer(*layer);
        }

        vp->prepareRT();

        m_uiViewport = vp;
        ViewportRegistry::get().addViewport(m_uiViewport);
    }

    {
        auto vp = std::make_shared<model::Viewport>(
            "player",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        vp->setUpdate([](model::Viewport& vp, const UpdateViewContext& ctx) {
            });

        vp->setBindBefore([this](model::Viewport& vp) {
            auto* buffer = m_playerRenderer->m_buffer.get();
            vp.setTexture(
                buffer->m_spec.attachments[LayerRenderer::ATT_ALBEDO_INDEX].textureID,
                buffer->m_spec.getSize());
            vp.setSourceFrameBuffer(buffer);
            });

        if (const auto* layer = LayerInfo::findLayer(LAYER_PLAYER); layer) {
            vp->applyLayer(*layer);
        }

        vp->prepareRT();

        m_playerViewport = vp;
        ViewportRegistry::get().addViewport(m_playerViewport);
    }

    {
        auto vp = std::make_shared<model::Viewport>(
            "Main",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        vp->setUpdate([](model::Viewport& vp, const UpdateViewContext& ctx) {
        });

        vp->setBindBefore([this](model::Viewport& vp) {
            auto* buffer = m_mainRenderer->m_buffer.get();
            vp.setTexture(
                buffer->m_spec.attachments[LayerRenderer::ATT_ALBEDO_INDEX].textureID,
                buffer->m_spec.getSize());
            vp.setSourceFrameBuffer(buffer);
        });


        if (const auto* layer = LayerInfo::findLayer(LAYER_MAIN); layer) {
            vp->applyLayer(*layer);
        }

        vp->prepareRT();

        m_mainViewport = vp;
        ViewportRegistry::get().addViewport(m_mainViewport);
    }

    if (assets.showRearView) {
        auto vp = std::make_shared<model::Viewport>(
            "Rear",
            glm::vec3(-1.f, -0.5f, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            true,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        vp->setBindBefore([this](model::Viewport& vp) {
            auto* buffer = m_rearRenderer->m_buffer.get();
            vp.setTexture(
                buffer->m_spec.attachments[LayerRenderer::ATT_ALBEDO_INDEX].textureID,
                buffer->m_spec.getSize());
            vp.setSourceFrameBuffer(buffer);
        });

        if (const auto* layer = LayerInfo::findLayer(LAYER_REAR); layer) {
            vp->applyLayer(*layer);
        }

        vp->prepareRT();

        m_rearViewport = vp;
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
    const auto& assets = ctx.getAssets();
    const auto& dbg = ctx.getDebug();

    NodeRegistry::get().makeSnapshotRT();

    //m_engine.getRegistry()->m_dispatcherView->dispatchEvents();

    m_collection->updateRT(ctx);
    m_engine.getRegistry()->updateRT(ctx);
}

void Scene::updateViewRT(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.getAssets();
    const auto& dbg = debug::DebugContext::get();

    {
        m_viewportRenderer->setGammaCorrectEnabled(dbg.m_gammaCorrectEnabled);
        m_viewportRenderer->setHardwareGammaEnabled(dbg.m_hardwareCorrectGammaEnabled);
        m_viewportRenderer->setHdrToneMappingEnabled(dbg.m_hdrToneMappingEnabled);
    }

    {
        if (auto* vp = m_uiViewport.get(); vp)
        {
            if (const auto* layer = LayerInfo::findLayer(LAYER_UI); layer) {
                vp->applyLayer(*layer);
            }
        }

        if (auto* vp = m_playerViewport.get(); vp)
        {
            if (const auto* layer = LayerInfo::findLayer(LAYER_PLAYER); layer) {
                vp->applyLayer(*layer);
            }
        }

        if (auto* vp = m_mainViewport.get(); vp)
        {
            if (const auto* layer = LayerInfo::findLayer(LAYER_MAIN); layer) {
                vp->applyLayer(*layer);
            }
        }

        if (auto* vp = m_rearViewport.get(); vp)
        {
            if (const auto* layer = LayerInfo::findLayer(LAYER_REAR); layer) {
                vp->applyLayer(*layer);
            }
        }
    }

    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->updateRT(ctx);
    }

    if (m_objectIdRenderer->isEnabled()) {
        m_objectIdRenderer->updateRT(ctx);
    }

    m_uiRenderer->updateRT(ctx);
    m_playerRenderer->updateRT(ctx);
    m_mainRenderer->updateRT(ctx);

    if (assets.showRearView) {
        m_rearRenderer->updateRT(ctx);
    }

    m_cubeMapRenderer->updateRT(ctx);
    m_mirrorMapRenderer->updateRT(ctx);
    m_waterMapRenderer->updateRT(ctx);

    //if (false)
    {
        const auto& spec = m_uiRenderer->m_buffer->m_spec;
        const glm::uvec2 aspectRatio = { spec.width, spec.height };
        if (aspectRatio != m_uiRenderer->m_aspectRatio) {
            m_uiRenderer->m_aspectRatio = aspectRatio;

            const auto* layer = LayerInfo::findLayer(LAYER_UI);
            if (layer && layer->m_enabled) {
                // NOTE KI tell WT that RT has diposed node
                event::Event evt{ event::Type::viewport_changed };
                auto& body = evt.body.view = {
                    .layer = layer->m_index,
                    .aspectRatio = aspectRatio,
                };
                m_engine.getRegistry()->m_dispatcherWorker->send(evt);
            }
        }
    }
}

void Scene::handleLoaded()
{
    m_loaded = true;

    //const auto& spec = m_uiRenderer->m_buffer->m_spec;
    //const glm::uvec2 aspectRatio = { spec.width, spec.height };
    //m_uiRenderer->m_aspectRatio = aspectRatio;

    //const auto* layer = LayerInfo::findLayer(LAYER_UI);
    //if (layer && layer->m_enabled) {
    //    event::Event evt{ event::Type::viewport_changed };
    //    auto& body = evt.body.view = {
    //        .layer = layer->m_index,
    //        .aspectRatio = aspectRatio,
    //    };
    //    m_engine.getRegistry()->m_dispatcherWorker->send(evt);
    //}
}

void Scene::handleNodeAdded(model::Node* node)
{
    if (!node) return;

    NodeRegistry::get().handleNodeAdded(node);
    m_collection->handleNodeAdded(node);
}

void Scene::handleNodeRemoved(model::Node* node)
{
    if (!node) return;

    NodeRegistry::get().handleNodeRemoved(node);
    m_collection->handleNodeRemoved(node);
}

void Scene::bind(const render::RenderContext& ctx)
{
    if (m_shadowMapRenderer->isEnabled()) {
        m_shadowMapRenderer->bind(ctx, m_shadowUBO);
    }

    NodeTypeRegistry::get().updateMaterials(ctx);
    NodeTypeRegistry::get().bindMaterials(ctx);
}

void Scene::unbind(const render::RenderContext& ctx)
{
}

void Scene::render(const render::RenderContext& ctx)
{
    const auto& assets = ctx.getAssets();
    auto& state = ctx.getGLState();

    state.setDepthFunc(ctx.m_depthFunc);

    bool wasCubeMap = false;
    int renderCount = 0;

    MaterialRegistry::get().renderMaterials(ctx);

    ctx.updateClipPlanesUBO();

    updateLightsUBO();

    if (m_shadowMapRenderer->render(ctx)) {
        renderCount++;
    }
    m_shadowMapRenderer->bindTexture(ctx.getGLState());

    updateShadowUBO();

    state.setEnabled(GL_TEXTURE_CUBE_MAP_SEAMLESS, assets.cubeMapSeamless);

    if (m_cubeMapRenderer->render(ctx)) {
        m_cubeMapRenderer->bindTexture(ctx.getGLState());
        m_waterMapRenderer->bindTexture(ctx.getGLState());
        m_mirrorMapRenderer->bindTexture(ctx.getGLState());

        wasCubeMap = assets.cubeMapSkipOthers;
    }
    wasCubeMap = false;

    if (!wasCubeMap && m_waterMapRenderer->render(ctx)) {
        m_cubeMapRenderer->bindTexture(ctx.getGLState());
        m_waterMapRenderer->bindTexture(ctx.getGLState());
        m_mirrorMapRenderer->bindTexture(ctx.getGLState());

        renderCount++;
    }

    if (!wasCubeMap && m_mirrorMapRenderer->render(ctx)) {
        m_cubeMapRenderer->bindTexture(ctx.getGLState());
        m_waterMapRenderer->bindTexture(ctx.getGLState());
        m_mirrorMapRenderer->bindTexture(ctx.getGLState());

        renderCount++;
    }

    {
        m_cubeMapRenderer->bindTexture(ctx.getGLState());
        m_waterMapRenderer->bindTexture(ctx.getGLState());
        m_mirrorMapRenderer->bindTexture(ctx.getGLState());
    }

    // NOTE KI skip main render if special update cycle
    //if (!wasCubeMap) // && renderCount <= 2)
    {
        // NOTE KI shadow map render causes first render here to produce garbage
        // => was visible in UI fps_counter, which was first
        renderPlayer(ctx);
        renderMain(ctx);
        renderRear(ctx);
        renderUi(ctx);
    }
    renderViewports(ctx);
}

void Scene::renderUi(const render::RenderContext& parentCtx)
{
    const auto* layer = LayerInfo::findLayer(LAYER_UI);
    if (!layer || !layer->m_enabled) return;

    render::Camera camera{};

    auto aspectRatio = (float)m_uiRenderer->m_buffer->m_spec.width /
        (float)m_uiRenderer->m_buffer->m_spec.height;

    float w = 4.f;
    float h = w;
    w = w * aspectRatio;
    float mw = w * 0.5f;
    float mh = h * 0.5f;
    camera.setViewport({-mw, mw, -mh, mh});

    // NOTE KI UI is "top level" context (i.e. main camera)
    render::RenderContext localCtx(
        "UI",
        nullptr,
        parentCtx.getClock(),
        m_engine.getRegistry(),
        m_collection.get(),
        m_engine.getRenderData(),
        //m_nodeDraw.get(),
        m_engine.getBatch(),
        &camera,
        0.1f,
        5.f,
        m_uiRenderer->m_buffer->m_spec.width,
        m_uiRenderer->m_buffer->m_spec.height,
        parentCtx.getDebug());

    localCtx.m_layer = layer->m_index;
    //localCtx.m_useLight = false;
    localCtx.m_useParticles = false;
    localCtx.m_useDecals = false;
    localCtx.m_useFog = false;
    localCtx.m_useEmission = false;
    localCtx.m_useSsao = false;
    localCtx.m_useBloom = false;
    localCtx.m_forceLineMode = false;
    localCtx.m_allowLineMode = false;
    //localCtx.m_useScreenspaceEffects = false;

    renderScene(localCtx, m_uiRenderer.get());
}

void Scene::renderPlayer(const render::RenderContext& parentCtx)
{
    const auto* layer = LayerInfo::findLayer(LAYER_PLAYER);
    if (!layer || !layer->m_enabled) return;

    render::RenderContext localCtx(
        "player",
        &parentCtx,
        parentCtx.m_camera,
        m_playerRenderer->m_buffer->m_spec.width,
        m_playerRenderer->m_buffer->m_spec.height);

    localCtx.m_layer = layer->m_index;
    localCtx.m_useParticles = false;
    localCtx.m_useDecals = false;
    localCtx.m_useFog = false;
    localCtx.m_useEmission = false;
    // TODO KI SSAO is so slow that cannot afford it multiple times
    localCtx.m_useSsao = false;
    localCtx.m_useBloom = false;
    //localCtx.m_useScreenspaceEffects = false;

    renderScene(localCtx, m_playerRenderer.get());
}

void Scene::renderMain(const render::RenderContext& parentCtx)
{
    const auto* layer = LayerInfo::findLayer(LAYER_MAIN);
    if (!layer || !layer->m_enabled) return;

    render::RenderContext localCtx(
        "MAIN",
        &parentCtx,
        parentCtx.m_camera,
        m_mainRenderer->m_buffer->m_spec.width,
        m_mainRenderer->m_buffer->m_spec.height);

    localCtx.m_layer = layer->m_index;

    localCtx.m_allowDrawDebug = true;
    renderScene(localCtx, m_mainRenderer.get());
}

// "back mirror" viewport
void Scene::renderRear(const render::RenderContext& parentCtx)
{
    const auto* layer = LayerInfo::findLayer(LAYER_REAR);
    if (!layer || !layer->m_enabled) return;

    const auto& assets = parentCtx.getAssets();

    if (!assets.showRearView) return;

    auto* parentCamera = parentCtx.m_camera;

    glm::vec3 cameraFront = parentCamera->getViewFront() * -1.f;

    render::Camera camera(
        parentCamera->getWorldPosition(),
        parentCamera->getViewFront(),
        parentCamera->getViewUp());
    camera.setViewport(parentCamera->getViewport());
    camera.setFov(parentCamera->getFov());
    camera.setAxis(cameraFront, parentCamera->getViewUp());

    render::RenderContext localCtx(
        "BACK",
        &parentCtx,
        &camera,
        m_rearRenderer->m_buffer->m_spec.width,
        m_rearRenderer->m_buffer->m_spec.height);

    localCtx.m_layer = 0; // LayerInfo::LAYER_MAIN;

    renderScene(localCtx, m_rearRenderer.get());
}

void Scene::renderViewports(const render::RenderContext& ctx)
{
    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->render(ctx, m_engine.getWindowBuffer());
    }
}

void Scene::renderScene(
    const render::RenderContext& ctx,
    LayerRenderer* layerRenderer)
{
    if (layerRenderer->isEnabled()) {
        ctx.updateUBOs();
        ctx.bindDefaults();

        auto* fb = layerRenderer->m_buffer.get();
        layerRenderer->render(ctx, fb);
    }
}

model::Node* Scene::getActiveNode() const
{
    return NodeRegistry::get().getActiveNode();
}

const std::vector<std::unique_ptr<NodeController>>* Scene::getActiveNodeControllers() const
{
    auto* node = getActiveNode();
    return node ? ControllerRegistry::get().forNode(node) : nullptr;
}

model::Node* Scene::getActiveCameraNode() const
{
    return m_collection->getActiveCameraNode();
}

const std::vector<std::unique_ptr<NodeController>>* Scene::getActiveCameraControllers() const
{
    auto* node = getActiveCameraNode();
    return node ? ControllerRegistry::get().forNode(node) : nullptr;
}

ki::node_id Scene::getObjectID(
    const render::RenderContext& ctx,
    float screenPosX,
    float screenPosY)
{
    if (m_objectIdRenderer->isEnabled()) {
        m_objectIdRenderer->render(ctx);
        return m_objectIdRenderer->getObjectId(ctx, screenPosX, screenPosY, m_mainViewport.get());
    }
    return 0;
}

void Scene::updateShadowUBO() const
{
    m_engine.getRenderData()->updateShadow(m_shadowUBO);
}

void Scene::updateLightsUBO() const
{
    m_engine.getRenderData()->updateLights(m_collection.get());
}

//void Scane::copyShadowMatrixFrom(const RenderContext& b)
//{
//    std::copy(
//        std::begin(b.m_cameraUBO.u_shadow),
//        std::end(b.m_cameraUBO.u_shadow),
//        std::begin(m_cameraUBO.u_shadow));
//
//    //std::copy(
//    //    std::begin(b.m_matrices.u_shadowProjected),
//    //    std::end(b.m_matrices.u_shadowProjected),
//    //    std::begin(m_matrices.u_shadowProjected));
//}
