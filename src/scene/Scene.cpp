#include "Scene.h"

#include <thread>
#include <mutex>

#include "ki/Timer.h"
#include "kigl/kigl.h"

#include "asset/UBO.h"
#include "asset/Shader.h"
#include "asset/DynamicCubeMap.h"

#include "backend/DrawBuffer.h"

#include "component/ParticleGenerator.h"

#include "model/Viewport.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"
#include "command/CommandAPI.h"
#include "command/api/InvokeLuaFunction.h"

#include "controller/NodeController.h"

#include "physics/PhysicsEngine.h"

#include "registry/Registry.h"
#include "registry/MaterialRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"

#include "engine/UpdateContext.h"

#include "render/NodeDraw.h"
#include "render/Batch.h"
#include "render/CubeMap.h"
#include "render/RenderContext.h"
#include "render/WindowBuffer.h"
#include "render/FrameBuffer.h"
#include "render/RenderData.h"

#include "renderer/ShadowCascade.h"

#include "renderer/NodeRenderer.h"
#include "renderer/ViewportRenderer.h"

#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/NormalRenderer.h"

#include "scene/ParticleSystem.h"

Scene::Scene(
    const Assets& assets,
    std::shared_ptr<Registry> registry)
    : m_assets(assets),
    m_alive(std::make_shared<std::atomic<bool>>(true)),
    m_registry(registry)
{
    m_commandEngine = std::make_unique<CommandEngine>(assets);
    m_scriptEngine = std::make_unique<ScriptEngine>(assets);

    {
        m_mainRenderer = std::make_unique<NodeRenderer>(true);
        m_rearRenderer = std::make_unique<NodeRenderer>(true);

        m_viewportRenderer = std::make_unique<ViewportRenderer>(true);

        m_waterMapRenderer = std::make_unique<WaterMapRenderer>(true, true, false);
        m_mirrorMapRenderer = std::make_unique<MirrorMapRenderer>(true, true, false);
        m_cubeMapRenderer = std::make_unique<CubeMapRenderer>(true);
        m_shadowMapRenderer = std::make_unique<ShadowMapRenderer>(true);

        m_objectIdRenderer = std::make_unique<ObjectIdRenderer>(false);
        m_normalRenderer = std::make_unique<NormalRenderer>(false);

        m_mainRenderer->setEnabled(true);
        m_rearRenderer->setEnabled(true);

        m_viewportRenderer->setEnabled(true);

        m_waterMapRenderer->setEnabled(m_assets.waterMapEnabled);
        m_mirrorMapRenderer->setEnabled(m_assets.mirrorMapEnabled);
        m_cubeMapRenderer->setEnabled(m_assets.cubeMapEnabled);
        m_shadowMapRenderer->setEnabled(m_assets.shadowMapEnabled);

        m_objectIdRenderer->setEnabled(true);
        m_normalRenderer->setEnabled(m_assets.showNormals);
    }

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
    m_registry->m_dispatcher->addListener(
        event::Type::node_added,
        [this](const event::Event& e) {
            this->bindComponents(e.body.node.target);
        });

    m_registry->m_dispatcher->addListener(
        event::Type::scene_loaded,
        [this](const event::Event& e) {
            m_loaded = true;
            this->m_registry->m_physicsEngine->setEnabled(true);
        });

    m_renderData->prepare();

    auto* registry = m_registry.get();

    m_commandEngine->prepare(registry);
    m_scriptEngine->prepare(m_commandEngine.get());

    m_batch->prepare(m_assets, registry);
    m_nodeDraw->prepare(m_assets, registry);

    m_mainRenderer->prepare(m_assets, registry);

    // NOTE KI OpenGL does NOT like interleaved draw and prepare
    if (m_rearRenderer->isEnabled()) {
        m_rearRenderer->prepare(m_assets, registry);
    }
    if (m_rearRenderer->isEnabled()) {
        m_rearRenderer->prepare(m_assets, registry);
    }

    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->prepare(m_assets, registry);
    }

    if (m_waterMapRenderer->isEnabled()) {
        m_waterMapRenderer->prepare(m_assets, registry);
    }
    if (m_mirrorMapRenderer->isEnabled()) {
        m_mirrorMapRenderer->prepare(m_assets, registry);
    }
    if (m_cubeMapRenderer->isEnabled()) {
        m_cubeMapRenderer->prepare(m_assets, registry);
    }
    if (m_shadowMapRenderer->isEnabled()) {
        m_shadowMapRenderer->prepare(m_assets, registry);
    }

    if (m_objectIdRenderer->isEnabled()) {
        m_objectIdRenderer->prepare(m_assets, registry);
    }

    if (m_normalRenderer->isEnabled()) {
        m_normalRenderer->prepare(m_assets, registry);
    }

    if (m_particleSystem) {
        m_particleSystem->prepare(m_assets, registry);
    }

    {
        m_windowBuffer = std::make_unique<WindowBuffer>(true);
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
            m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

        m_mainViewport->setUpdate([](Viewport& vp, const UpdateContext& ctx) {
        });

        m_mainViewport->setBindBefore([this](Viewport& vp) {
            auto* buffer = m_mainRenderer->m_buffer.get();
            vp.setTextureId(buffer->m_spec.attachments[NodeRenderer::ATT_ALBEDO_INDEX].textureID);
            vp.setSourceFrameBuffer(buffer);
        });

        m_mainViewport->setGammaCorrect(true);
        m_mainViewport->setHardwareGamma(true);

        m_mainViewport->setEffectEnabled(m_assets.viewportEffectEnabled);
        m_mainViewport->setEffect(m_assets.viewportEffect);

        m_mainViewport->prepare(m_assets);
        m_registry->m_viewportRegistry->addViewport(m_mainViewport);
    }

    if (m_assets.showRearView) {
        m_rearViewport = std::make_shared<Viewport>(
            "Rear",
            glm::vec3(-1.f, -0.5f, 0),
            glm::vec3(0, 0, 0),
            glm::vec2(0.5f, 0.5f),
            true,
            0,
            m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));

        m_rearViewport->setBindBefore([this](Viewport& vp) {
            auto* buffer = m_rearRenderer->m_buffer.get();
            vp.setTextureId(buffer->m_spec.attachments[NodeRenderer::ATT_ALBEDO_INDEX].textureID);
            vp.setSourceFrameBuffer(buffer);
        });

        m_rearViewport->setGammaCorrect(true);
        m_rearViewport->setHardwareGamma(true);

        m_rearViewport->prepare(m_assets);
        m_registry->m_viewportRegistry->addViewport(m_rearViewport);
    }

    if (m_assets.showObjectIDView) {
        if (m_objectIdRenderer->isEnabled()) {
            m_registry->m_viewportRegistry->addViewport(m_objectIdRenderer->m_debugViewport);
        }
    }

    if (m_assets.showShadowMapView) {
        if (m_shadowMapRenderer->isEnabled()) {
            m_registry->m_viewportRegistry->addViewport(m_shadowMapRenderer->m_debugViewport);
        }
    }
    if (m_assets.showReflectionView) {
        if (m_waterMapRenderer->isEnabled()) {
            m_registry->m_viewportRegistry->addViewport(m_waterMapRenderer->m_reflectionDebugViewport);
        }
        if (m_mirrorMapRenderer->isEnabled()) {
            m_registry->m_viewportRegistry->addViewport(m_mirrorMapRenderer->m_reflectionDebugViewport);
        }
    }
    if (m_assets.showRefractionView) {
        if (m_waterMapRenderer->isEnabled()) {
            m_registry->m_viewportRegistry->addViewport(m_waterMapRenderer->m_refractionDebugViewport);
        }
    }
}

void Scene::processEvents(const UpdateContext& ctx)
{
    m_registry->m_dispatcher->dispatchEvents(ctx);
}

void Scene::update(const UpdateContext& ctx)
{
    //if (ctx.clock.frameCount > 120) {
    if (m_loaded) {
        m_commandEngine->update(ctx);
    }

    if (auto root = m_registry->m_nodeRegistry->m_root) {
        root->update(ctx);
        m_registry->m_physicsEngine->update(ctx);
    }

    for (auto& generator : m_particleGenerators) {
        generator->update(ctx);
    }

    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->update(ctx);
    }

    if (m_particleSystem) {
        m_particleSystem->update(ctx);
    }

    m_registry->update(ctx);

    m_renderData->update();
}

void Scene::updateView(const RenderContext& ctx)
{
    if (m_objectIdRenderer->isEnabled()) {
        m_objectIdRenderer->updateView(ctx);
    }

    m_mainRenderer->updateView(ctx);

    if (m_assets.showRearView) {
        m_rearRenderer->updateView(ctx);
    }

    m_cubeMapRenderer->updateView(ctx);
    m_mirrorMapRenderer->updateView(ctx);
    m_waterMapRenderer->updateView(ctx);

    m_nodeDraw->updateView(ctx);
    m_windowBuffer->updateView(ctx);
}

void Scene::bind(const RenderContext& ctx)
{
    if (m_shadowMapRenderer->isEnabled()) {
        m_shadowMapRenderer->bind(ctx);
    }

    m_registry->m_typeRegistry->bind(ctx);

    m_renderData->bind();

    ctx.m_data.u_cubeMapExist = m_cubeMapRenderer->isEnabled() && m_cubeMapRenderer->isRendered();

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
    ctx.m_state.setDepthFunc(ctx.m_depthFunc);

    bool wasCubeMap = false;
    int renderCount = 0;

    if (m_shadowMapRenderer->isEnabled() && m_shadowMapRenderer->render(ctx)) {
        renderCount++;
        m_shadowMapRenderer->bindTexture(ctx);
    }

    ctx.m_state.setEnabled(GL_TEXTURE_CUBE_MAP_SEAMLESS, ctx.m_assets.cubeMapSeamless);

    if (m_cubeMapRenderer->isEnabled() && m_cubeMapRenderer->render(ctx)) {
        wasCubeMap = ctx.m_assets.cubeMapSkipOthers;
    }

    if (!wasCubeMap && m_waterMapRenderer->isEnabled() && m_waterMapRenderer->render(ctx))
        renderCount++;

    if (!wasCubeMap && m_mirrorMapRenderer->isEnabled() && m_mirrorMapRenderer->render(ctx))
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
    if (!m_assets.showRearView) return;

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
    m_registry->m_materialRegistry->bind(ctx);
    m_registry->m_spriteRegistry->bind(ctx);
    m_registry->m_entityRegistry->bind(ctx);

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

    //targetBuffer->bind(ctx);

    if (m_particleSystem) {
        m_particleSystem->render(ctx);
    }

    if (m_assets.showNormals) {
        if (m_normalRenderer->isEnabled()) {
            m_normalRenderer->render(ctx);
        }
    }
}

Node* Scene::getActiveNode() const
{
    return m_registry->m_nodeRegistry->getActiveNode();
}

const std::vector<NodeController*>* Scene::getActiveNodeControllers() const
{
    auto* node = getActiveNode();
    return node ? m_registry->m_controllerRegistry->getControllers(node) : nullptr;
}

Node* Scene::getActiveCamera() const
{
    return m_registry->m_nodeRegistry->getActiveCamera2();
}

const std::vector<NodeController*>* Scene::getActiveCameraControllers() const
{
    auto* node = getActiveCamera();
    return node ? m_registry->m_controllerRegistry->getControllers(node) : nullptr;
}

void Scene::bindComponents(Node* node)
{
    auto& type = node->m_type;

    if (node->m_particleGenerator) {
        if (m_particleSystem) {
            node->m_particleGenerator->setSystem(m_particleSystem.get());
            m_particleGenerators.push_back(node);
        }
    }

    if (m_assets.useScript) {
        m_scriptEngine->registerNode(node);
        m_scriptEngine->registerScript(node, NodeScriptId::init, type->m_script);

        m_scriptEngine->runScript(node, NodeScriptId::init);

        // NOTE KI start via queue, in sync with next update cycle
        if (m_scriptEngine->hasFunction(node, "start")) {
            m_commandEngine->addCommand(
                std::make_unique<InvokeLuaFunction>(
                    -1,
                    node->m_id,
                    "start"));
        }
    }
}

ki::object_id Scene::getObjectID(const RenderContext& ctx, float screenPosX, float screenPosY)
{
    if (m_objectIdRenderer->isEnabled()) {
        m_objectIdRenderer->render(ctx);
        return m_objectIdRenderer->getObjectId(ctx, screenPosX, screenPosY, m_mainViewport.get());
    }
    return 0;
}
