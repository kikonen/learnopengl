#include "Scene.h"

#include <iostream>

#include "kigl/GLState.h"

#include "util/thread.h"

#include "asset/Assets.h"
#include "asset/DynamicCubeMap.h"

#include "shader/Shader.h"
#include "shader/LightsUBO.h"

#include "model/Viewport.h"

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
#include "render/PassSsao.h"

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
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_alive{ alive },
    m_registry{ registry },
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

    m_batch = std::make_unique<render::Batch>();
    //m_nodeDraw = std::make_unique<render::NodeDraw>();
    m_renderData = std::make_unique<render::RenderData>();
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
}

void Scene::prepareRT()
{
    const auto& assets = Assets::get();
    const auto& dbg = debug::DebugContext::get();

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

    dispatcherView->addListener(
        event::Type::node_removed,
        [this](const event::Event& e) {
            auto* node = pool::NodeHandle::toNode(e.body.node.target);
            this->handleNodeRemoved(node);

            // NOTE KI tell WT that RT has diposed node
            event::Event evt{ event::Type::node_dispose };
            auto& body = evt.body.node = {
                .target = e.body.node.target,
            };
            m_registry->m_dispatcherWorker->send(evt);
        });

    dispatcherView->addListener(
        event::Type::camera_activate,
        [this](const event::Event& e) {
            auto& data = e.body.node;
            auto nodeHandle = pool::NodeHandle::toHandle(data.target);
            m_collection->setActiveCameraNode(nodeHandle);
        });

    dispatcherView->addListener(
        event::Type::camera_activate_next,
        [this](const event::Event& e) {
            auto& data = e.body.node;
            auto nodeHandle = pool::NodeHandle::toHandle(data.target);
            auto nextCamera = m_collection->getNextCameraNode(nodeHandle, data.offset);
            m_collection->setActiveCameraNode(nextCamera);
        });

    m_renderData->prepare(
        false,
        assets.glUseInvalidate,
        assets.glUseFence,
        assets.glUseFenceDebug,
        assets.batchDebug);

    PrepareContext ctx{ m_registry.get() };

    m_batch->prepareRT(ctx);
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
        m_windowBuffer = std::make_unique<render::WindowBuffer>(true);
    }

    {
        auto vp = std::make_shared<Viewport>(
            "UI",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        vp->setUpdate([](Viewport& vp, const UpdateViewContext& ctx) {
            });

        vp->setBindBefore([this](Viewport& vp) {
            auto* buffer = m_uiRenderer->m_buffer.get();
            vp.setTextureId(buffer->m_spec.attachments[LayerRenderer::ATT_ALBEDO_INDEX].textureID);
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
        auto vp = std::make_shared<Viewport>(
            "player",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        vp->setUpdate([](Viewport& vp, const UpdateViewContext& ctx) {
            });

        vp->setBindBefore([this](Viewport& vp) {
            auto* buffer = m_playerRenderer->m_buffer.get();
            vp.setTextureId(buffer->m_spec.attachments[LayerRenderer::ATT_ALBEDO_INDEX].textureID);
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
        auto vp = std::make_shared<Viewport>(
            "Main",
            //glm::vec3(-0.75, 0.75, 0),
            glm::vec3(-1.0f, 1.f, 0),
            glm::vec3(0, 0, 0),
            //glm::vec2(1.5f, 1.5f),
            glm::vec2(2.f, 2.f),
            false,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        vp->setUpdate([](Viewport& vp, const UpdateViewContext& ctx) {
        });

        vp->setBindBefore([this](Viewport& vp) {
            auto* buffer = m_mainRenderer->m_buffer.get();
            vp.setTextureId(buffer->m_spec.attachments[LayerRenderer::ATT_ALBEDO_INDEX].textureID);
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
        auto vp = std::make_shared<Viewport>(
            "Rear",
            glm::vec3(-1.f, -0.5f, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            true,
            0,
            ProgramRegistry::get().getProgram(SHADER_VIEWPORT));

        vp->setBindBefore([this](Viewport& vp) {
            auto* buffer = m_rearRenderer->m_buffer.get();
            vp.setTextureId(buffer->m_spec.attachments[LayerRenderer::ATT_ALBEDO_INDEX].textureID);
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
    const auto& assets = ctx.m_assets;
    const auto& dbg = ctx.m_dbg;

    // NOTE KI race condition with program prepare and event processing
    // NOTE KI also rece with snapshot and event processing
    // => doing programs before snapshot reduce scope
    //    but DOES NOT remove it
    ProgramRegistry::get().updateRT(ctx);

    NodeRegistry::get().prepareUpdateRT(ctx);

    m_registry->m_dispatcherView->dispatchEvents();

    m_collection->updateRT(ctx);
    m_registry->updateRT(ctx);

    m_batch->updateRT(ctx);
}

void Scene::postRT(const UpdateContext& ctx)
{
    m_registry->postRT(ctx);
}

void Scene::updateViewRT(const UpdateViewContext& ctx)
{
    const auto& assets = ctx.m_assets;
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

    //m_nodeDraw->updateRT(ctx);
    m_windowBuffer->updateRT(ctx);

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
                m_registry->m_dispatcherWorker->send(evt);
            }
        }
    }
}

void Scene::handleNodeAdded(Node* node)
{
    if (!node) return;

    NodeRegistry::get().handleNodeAdded(node);
    m_collection->handleNodeAdded(node);
}

void Scene::handleNodeRemoved(Node* node)
{
    if (!node) return;

    NodeRegistry::get().handleNodeRemoved(node);
    m_collection->handleNodeRemoved(node);
}

void Scene::bind(const RenderContext& ctx)
{
    prepareUBOs(ctx);

    if (m_shadowMapRenderer->isEnabled()) {
        m_shadowMapRenderer->bind(ctx, m_shadowUBO);
    }

    NodeTypeRegistry::get().updateMaterials(ctx);
    NodeTypeRegistry::get().bindMaterials(ctx);

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

void Scene::render(const RenderContext& ctx)
{
    const auto& assets = ctx.m_assets;
    auto& state = ctx.m_state;

    state.setDepthFunc(ctx.m_depthFunc);

    bool wasCubeMap = false;
    int renderCount = 0;

    MaterialRegistry::get().renderMaterials(ctx);

    updateUBOs();
    ctx.updateClipPlanesUBO();

    if (m_shadowMapRenderer->render(ctx)) {
        renderCount++;
        m_shadowMapRenderer->bindTexture(ctx.m_state);
    }

    updateShadowUBO();

    state.setEnabled(GL_TEXTURE_CUBE_MAP_SEAMLESS, assets.cubeMapSeamless);

    if (m_cubeMapRenderer->render(ctx)) {
        m_cubeMapRenderer->bindTexture(ctx.m_state);
        m_waterMapRenderer->bindTexture(ctx.m_state);
        m_mirrorMapRenderer->bindTexture(ctx.m_state);

        wasCubeMap = assets.cubeMapSkipOthers;
    }
    wasCubeMap = false;

    if (!wasCubeMap && m_waterMapRenderer->render(ctx)) {
        m_cubeMapRenderer->bindTexture(ctx.m_state);
        m_waterMapRenderer->bindTexture(ctx.m_state);
        m_mirrorMapRenderer->bindTexture(ctx.m_state);

        renderCount++;
    }

    if (!wasCubeMap && m_mirrorMapRenderer->render(ctx)) {
        m_cubeMapRenderer->bindTexture(ctx.m_state);
        m_waterMapRenderer->bindTexture(ctx.m_state);
        m_mirrorMapRenderer->bindTexture(ctx.m_state);

        renderCount++;
    }

    {
        m_cubeMapRenderer->bindTexture(ctx.m_state);
        m_waterMapRenderer->bindTexture(ctx.m_state);
        m_mirrorMapRenderer->bindTexture(ctx.m_state);
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

    m_renderData->invalidateAll();
}

void Scene::renderUi(const RenderContext& parentCtx)
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
    RenderContext localCtx(
        "UI",
        nullptr,
        parentCtx.m_clock,
        m_registry.get(),
        m_collection.get(),
        m_renderData.get(),
        //m_nodeDraw.get(),
        m_batch.get(),
        &camera,
        0.1f,
        5.f,
        m_uiRenderer->m_buffer->m_spec.width,
        m_uiRenderer->m_buffer->m_spec.height,
        parentCtx.m_dbg);

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

void Scene::renderPlayer(const RenderContext& parentCtx)
{
    const auto* layer = LayerInfo::findLayer(LAYER_PLAYER);
    if (!layer || !layer->m_enabled) return;

    RenderContext localCtx(
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

void Scene::renderMain(const RenderContext& parentCtx)
{
    const auto* layer = LayerInfo::findLayer(LAYER_MAIN);
    if (!layer || !layer->m_enabled) return;

    RenderContext localCtx(
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
void Scene::renderRear(const RenderContext& parentCtx)
{
    const auto* layer = LayerInfo::findLayer(LAYER_REAR);
    if (!layer || !layer->m_enabled) return;

    const auto& assets = parentCtx.m_assets;

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

    RenderContext localCtx(
        "BACK",
        &parentCtx,
        &camera,
        m_rearRenderer->m_buffer->m_spec.width,
        m_rearRenderer->m_buffer->m_spec.height);

    localCtx.m_layer = 0; // LayerInfo::LAYER_MAIN;

    renderScene(localCtx, m_rearRenderer.get());
}

void Scene::renderViewports(const RenderContext& ctx)
{
    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->render(ctx, m_windowBuffer.get());
    }
}

void Scene::renderScene(
    const RenderContext& ctx,
    LayerRenderer* layerRenderer)
{
    if (layerRenderer->isEnabled()) {
        ctx.updateUBOs();
        ctx.bindDefaults();

        auto* fb = layerRenderer->m_buffer.get();
        layerRenderer->render(ctx, fb);
    }
}

Node* Scene::getActiveNode() const
{
    return NodeRegistry::get().getActiveNode();
}

const std::vector<std::unique_ptr<NodeController>>* Scene::getActiveNodeControllers() const
{
    auto* node = getActiveNode();
    return node ? ControllerRegistry::get().forNode(node) : nullptr;
}

Node* Scene::getActiveCameraNode() const
{
    return m_collection->getActiveCameraNode();
}

const std::vector<std::unique_ptr<NodeController>>* Scene::getActiveCameraControllers() const
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

void Scene::prepareUBOs(const RenderContext& ctx)
{
    //KI_INFO_OUT(fmt::format("ts: {}", m_data.u_time));
    const debug::DebugContext& dbg = ctx.m_dbg;
    auto& assets = ctx.m_assets;
    auto& selectionRegistry = *m_registry->m_selectionRegistry;

    auto cubeMapEnabled = dbg.m_cubeMapEnabled &&
        m_cubeMapRenderer->isEnabled() &&
        m_cubeMapRenderer->isRendered();

    m_dataUBO = {
        dbg.m_fogColor,
        // NOTE KI keep original screen resolution across the board
        // => current buffer resolution is separately in bufferInfo UBO
        //m_parent ? m_parent->m_resolution : m_resolution,

        selectionRegistry.getSelectionMaterialIndex(),
        selectionRegistry.getTagMaterialIndex(),

        cubeMapEnabled,
        assets.skyboxEnabled,

        assets.environmentMapEnabled,

        dbg.m_shadowVisual,
        dbg.m_forceLineMode,

        dbg.m_fogStart,
        dbg.m_fogEnd,
        dbg.m_fogDensity,

        dbg.m_effectOitMinBlendThreshold,
        dbg.m_effectOitMaxBlendThreshold,

        dbg.m_effectBloomThreshold,

        dbg.m_gammaCorrect,
        dbg.m_hdrExposure,

        static_cast<float>(ctx.m_clock.ts),
        static_cast<int>(ctx.m_clock.frameCount),
    };

    for (int i = 0; const auto& v : render::PassSsao::getKernel()) {
        if (i >= 64) break;
        m_dataUBO.u_ssaoSamples[i++] = v;
    }

    {
        float parallaxDepth = -1.f;
        if (!dbg.m_parallaxEnabled) {
            parallaxDepth = 0;
        }
        else if (dbg.m_parallaxDebugEnabled) {
            parallaxDepth = dbg.m_parallaxDebugDepth;
        }

        m_debugUBO = {
            dbg.m_wireframeLineColor,
            dbg.m_skyboxColor,
            dbg.m_effectSsaoBaseColor,

            dbg.m_wireframeOnly,
            dbg.m_wireframeLineWidth,

            dbg.m_entityId,
            dbg.m_animation.m_boneIndex,
            dbg.m_animation.m_debugBoneWeight,

            dbg.m_lightEnabled,
            dbg.m_normalMapEnabled,

            dbg.m_skyboxColorEnabled,

            dbg.m_effectSsaoEnabled,
            dbg.m_effectSsaoBaseColorEnabled,

            parallaxDepth,
            dbg.m_parallaxMethod,
        };
    }
}

void Scene::updateUBOs() const
{
    // https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs
    updateDataUBO();
    updateDebugUBO();
    updateLightsUBO();
}

void Scene::updateDataUBO() const
{
    m_renderData->updateData(m_dataUBO);
}

void Scene::updateShadowUBO() const
{
    m_renderData->updateShadow(m_shadowUBO);
}

void Scene::updateDebugUBO() const
{
    m_renderData->updateDebug(m_debugUBO);
}

void Scene::updateLightsUBO() const
{
    m_renderData->updateLights(m_collection.get());
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
