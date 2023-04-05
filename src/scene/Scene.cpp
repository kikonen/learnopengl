#include "Scene.h"

#include <thread>
#include <mutex>

#include "ki/GL.h"
#include "ki/Timer.h"

#include "asset/UBO.h"
#include "asset/Shader.h"
#include "asset/DynamicCubeMap.h"

#include "backend/DrawBuffer.h"

#include "component/ParticleGenerator.h"

#include "model/Viewport.h"

#include "command/CommandEngine.h"
#include "command/ScriptEngine.h"
#include "command/CommandAPI.h"
#include "command/api/ResumeNode.h"

#include "controller/NodeController.h"

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
        m_nodeRenderer = std::make_unique<NodeRenderer>();
        m_viewportRenderer = std::make_unique<ViewportRenderer>();

        m_waterMapRenderer = std::make_unique<WaterMapRenderer>();
        m_mirrorMapRenderer = std::make_unique<MirrorMapRenderer>();
        m_cubeMapRenderer = std::make_unique<CubeMapRenderer>();
        m_shadowMapRenderer = std::make_unique<ShadowMapRenderer>();

        m_objectIdRenderer = std::make_unique<ObjectIdRenderer>();
        m_normalRenderer = std::make_unique<NormalRenderer>();

        m_nodeRenderer->setEnabled(true);
        m_viewportRenderer->setEnabled(true);

        m_waterMapRenderer->setEnabled(m_assets.renderWaterMap);
        m_mirrorMapRenderer->setEnabled(m_assets.renderMirrorMap);
        m_cubeMapRenderer->setEnabled(m_assets.renderCubeMap);
        m_shadowMapRenderer->setEnabled(m_assets.renderShadowMap);

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

    m_renderData->prepare();

    auto* registry = m_registry.get();

    m_commandEngine->prepare(registry);
    m_scriptEngine->prepare(m_commandEngine.get());

    m_batch->prepare(m_assets, registry);
    m_nodeDraw->prepare(m_assets, registry);

    // NOTE KI OpenGL does NOT like interleaved draw and prepare
    if (m_nodeRenderer->isEnabled()) {
        m_nodeRenderer->prepare(m_assets, registry);
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

        m_registry->m_viewportRegistry->addViewport(m_mainViewport);

        m_registry->m_physicsEngine->prepare();
    }

    if (m_assets.showObjectIDView) {
        if (m_objectIdRenderer->isEnabled()) {
            m_registry->m_viewportRegistry->addViewport(m_objectIdRenderer->m_debugViewport);
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
        m_registry->m_viewportRegistry->addViewport(m_rearViewport);
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
            m_registry->m_viewportRegistry->addViewport(m_mirrorMapRenderer->m_debugViewport);
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
    if (getActiveCamera()) {
        m_commandEngine->update(ctx);
    }

    if (m_registry->m_nodeRegistry->m_root) {
        m_registry->m_nodeRegistry->m_root->update(ctx);
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

    updateMainViewport(ctx);

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

    // OpenGL Programming Guide, 8th Edition, page 404
    // Enable polygon offset to resolve depth-fighting isuses
    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(0.2f, 0.2f);
    ctx.m_state.setEnabled(GL_TEXTURE_CUBE_MAP_SEAMLESS, true);

    if (m_cubeMapRenderer->isEnabled() && m_cubeMapRenderer->render(ctx)) {
        //wasCubeMap = true;
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

    //glDisable(GL_POLYGON_OFFSET_FILL);
}

void Scene::drawMain(const RenderContext& parentCtx)
{
    RenderContext localCtx(
        "MAIN",
        &parentCtx,
        parentCtx.m_camera,
        m_mainBuffer->m_spec.width,
        m_mainBuffer->m_spec.height);

    localCtx.copyShadowFrom(parentCtx);

    drawScene(localCtx, m_mainBuffer.get());
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

    localCtx.copyShadowFrom(parentCtx);

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

    if (m_viewportRenderer->isEnabled()) {
        m_viewportRenderer->render(ctx, m_windowBuffer.get());
    }
}

void Scene::drawScene(
    const RenderContext& ctx,
    FrameBuffer* targetBuffer)
{
    m_registry->m_materialRegistry->bind(ctx);
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

    if (m_nodeRenderer->isEnabled()) {
        m_nodeRenderer->render(ctx, targetBuffer);
    }

    targetBuffer->bind(ctx);

    if (m_particleSystem) {
        m_particleSystem->render(ctx);
    }

    if (m_assets.showNormals) {
        if (m_normalRenderer->isEnabled()) {
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
    return node ? m_registry->m_controllerRegistry->get<NodeController>(node) : nullptr;
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
                std::make_unique<ResumeNode>(
                    -1,
                    node->m_objectID,
                    "start"));
        }
    }
}

int Scene::getObjectID(const RenderContext& ctx, double screenPosX, double screenPosY)
{
    if (m_objectIdRenderer->isEnabled()) {
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
        auto buffer = new FrameBuffer(
            "main",
            {
                w, h,
                { FrameBufferAttachment::getTextureRGBA(), FrameBufferAttachment::getRBODepthStencil() }
            });

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
            auto buffer = new FrameBuffer(
                "rear",
                {
                    mirrorW, mirrorH,
                    { FrameBufferAttachment::getTextureRGBA(), FrameBufferAttachment::getRBODepthStencil() }
                });

            m_rearBuffer.reset(buffer);
            m_rearBuffer->prepare(true, { 0, 0, 0, 0.0 });

            m_rearViewport->setTextureId(m_rearBuffer->m_spec.attachments[0].textureID);
            m_rearViewport->setSourceFrameBuffer(m_rearBuffer.get());
        }
    }
}
