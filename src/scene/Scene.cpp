#include "Scene.h"

#include <thread>
#include <mutex>

#include "ki/GL.h"
#include "ki/Timer.h"

#include "asset/UBO.h"

#include "RenderData.h"

#include "controller/NodeController.h"


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
    m_cubeMapRenderer = std::make_unique<CubeMapRenderer>();
    m_shadowMapRenderer = std::make_unique<ShadowMapRenderer>();

    m_objectIdRenderer = std::make_unique<ObjectIdRenderer>();
    m_normalRenderer = std::make_unique<NormalRenderer>();

    particleSystem = std::make_unique<ParticleSystem>();

    m_renderData = std::make_unique<RenderData>();
}

Scene::~Scene()
{
    KI_INFO("SCENE: deleted");

    particleGenerators.clear();
}

void Scene::prepare(ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    m_renderData->prepare();

    m_commandEngine.prepare(assets);
    m_scriptEngine.prepare(assets, m_commandEngine);

    m_batch.prepare(assets, assets.batchSize);
    m_materialRegistry.prepare();
    m_modelRegistry.prepare(m_batch);
    m_registry.prepare(&m_batch, &m_materialRegistry, &m_modelRegistry);

    // NOTE KI OpenGL does NOT like interleaved draw and prepare
    if (m_nodeRenderer) {
        m_nodeRenderer->prepare(assets, shaders, m_materialRegistry);
    }
    //terrainRenderer->prepare(shaders);

    if (m_viewportRenderer) {
        m_viewportRenderer->prepare(assets, shaders, m_materialRegistry);
    }

    if (m_waterMapRenderer) {
        m_waterMapRenderer->prepare(assets, shaders, m_materialRegistry);
    }
    if (m_mirrorMapRenderer) {
        m_mirrorMapRenderer->prepare(assets, shaders, m_materialRegistry);
    }
    if (m_cubeMapRenderer) {
        m_cubeMapRenderer->prepare(assets, shaders, m_materialRegistry);
    }
    if (m_shadowMapRenderer) {
        m_shadowMapRenderer->prepare(assets, shaders, m_materialRegistry);
    }

    if (m_objectIdRenderer) {
        m_objectIdRenderer->prepare(assets, shaders, m_materialRegistry);
    }

    if (assets.showNormals) {
        if (m_normalRenderer) {
            m_normalRenderer->prepare(assets, shaders, m_materialRegistry);
        }
    }

    if (particleSystem) {
        particleSystem->prepare(assets, shaders);
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
            true,
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
            true,
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

    m_windowBuffer->update(ctx);

    m_renderData->update();
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

    m_renderData->bind();

    ctx.bindDefaults();
    ctx.bindUBOs();

    m_batch.bind();
}


void Scene::unbind(RenderContext& ctx)
{
}

void Scene::draw(RenderContext& ctx)
{
    glDepthFunc(GL_LEQUAL);

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

    RenderContext mirrorCtx("BACK", &ctx, camera, m_rearBuffer->m_spec.width, m_rearBuffer->m_spec.height);
    mirrorCtx.m_matrices.lightProjected = ctx.m_matrices.lightProjected;
    mirrorCtx.bindMatricesUBO();

    m_rearBuffer->bind(mirrorCtx);

    drawScene(mirrorCtx);

    m_rearBuffer->unbind(ctx);
    ctx.bindMatricesUBO();
}

void Scene::drawViewports(RenderContext& ctx)
{
    m_windowBuffer->bind(ctx);

    // NOTE KI this clears *window* buffer, not actual "main" buffer used for drawing
    // => Stencil is not supposed to exist here
    // => no need to clear this; ViewPort will do glBlitNamedFramebuffer
    // => *BUT* if glDraw is used instead then clear *IS* needed for depth
    if (false) {
        int mask = GL_DEPTH_BUFFER_BIT;
        if (assets.clearColor) {
            if (assets.debugClearColor) {
                //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClearColor(0.9f, 0.9f, 0.0f, 1.0f);
            }
            mask |= GL_COLOR_BUFFER_BIT;
        }
        glClear(mask);
    }

    if (m_viewportRenderer) {
        m_viewportRenderer->render(ctx, m_windowBuffer.get());
    }
}

void Scene::drawScene(RenderContext& ctx)
{
    // NOTE KI clear for current draw buffer buffer (main/mirror/etc.)
    {
        int mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        if (assets.clearColor) {
            if (assets.debugClearColor) {
                glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
            }
            mask |= GL_COLOR_BUFFER_BIT;
        }
        glClear(mask);
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

Camera* Scene::getCamera() const
{
    return !m_registry.m_cameraNodes.empty() ? m_registry.m_cameraNodes[0]->m_camera.get() : nullptr;
}

NodeController* Scene::getCameraController() const
{
    if (m_registry.m_cameraNodes.empty()) return nullptr;
    auto& node = m_registry.m_cameraNodes[0];
    return node->m_controller.get();
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
    KI_INFO(fmt::format("FRAME_BUFFER: update - w={}, h={}", w, h));

    // MAIN
    {
        // NOTE KI alpha NOT needed
        auto buffer = new TextureBuffer({
            w, h,
            { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepthStencil() } });

        m_mainBuffer.reset(buffer);
        m_mainBuffer->prepare(true, { 0, 0, 0, 1.0 });

        m_mainViewport->setTextureId(m_mainBuffer->m_spec.attachments[0].textureID);
        m_mainViewport->setSourceFrameBuffer(m_mainBuffer.get());
    }

    // VMIRROR
    {
        int mirrorW = w * 0.5;
        int mirrorH = h * 0.5;

        if (mirrorW < 1) mirrorW = 1;
        if (mirrorH < 1) mirrorH = 1;

        if (!m_rearBuffer && assets.showRearView) {
            // NOTE KI alpha NOT needed
            auto buffer = new TextureBuffer({
                mirrorW, mirrorH,
                { FrameBufferAttachment::getTextureRGB(), FrameBufferAttachment::getRBODepthStencil() } });

            m_rearBuffer.reset(buffer);
            m_rearBuffer->prepare(true, { 0, 0, 0, 1.0 });

            m_rearViewport->setTextureId(m_rearBuffer->m_spec.attachments[0].textureID);
            m_rearViewport->setSourceFrameBuffer(m_rearBuffer.get());
        }
    }
}

