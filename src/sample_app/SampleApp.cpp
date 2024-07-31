#include "SampleApp.h"

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "util/Util.h"

#include "kigl/kigl.h"

#include "ki/sid.h"

#include "pool/NodeHandle.h"

#include "editor/EditorFrame.h"

#include "asset/DynamicCubeMap.h"

#include "backend/gl/PerformanceCounters.h"

#include "controller/VolumeController.h"

#include "script/api/SetTextNode.h"
#include "script/CommandEngine.h"

#include "event/Dispatcher.h"

#include "audio/Source.h"
#include "audio/AudioEngine.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/NodeSnapshotRegistry.h"

#include "engine/AssetsLoader.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"
#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"

#include "loader/SceneLoader.h"

#include "scene/Scene.h"
#include "scene/SceneUpdater.h"
#include "scene/ParticleUpdater.h"
#include "scene/AnimationUpdater.h"

#include "TestSceneSetup.h"

#include "gui/Input.h"
#include "gui/Window.h"

#include "physics/PhysicsEngine.h"

namespace {
    const glm::vec4 BLACK_COLOR{ 0.f };

    ki::node_id fpsNodeId = SID("fps_counter");
}

SampleApp::SampleApp()
{
}

SampleApp::~SampleApp()
{
}

int SampleApp::onInit()
{
    m_title = "OpenGL";
    //glfwWindowHint(GLFW_SAMPLES, 4);

    Assets::set(loadAssets());

    {
        const auto& assets = Assets::get();
        auto& debugContext = render::DebugContext::modify();
        debugContext.m_frustumEnabled = assets.frustumEnabled;
        debugContext.m_forceWireframe = assets.forceWireframe;
        debugContext.m_showNormals = assets.showNormals;
    }

    return 0;
}

int SampleApp::onSetup()
{
    const auto& assets = Assets::get();

    m_currentScene = loadScene();

    m_sceneUpdater = std::make_shared<SceneUpdater>(
        m_registry,
        m_alive);

    m_particleUpdater = std::make_shared<ParticleUpdater>(
        m_registry,
        m_alive);

    m_animationUpdater = std::make_shared<AnimationUpdater>(
        m_registry,
        m_alive);

    m_sceneUpdater->start();
    m_particleUpdater->start();
    m_animationUpdater->start();

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (assets.glfwSwapInterval >= 0) {
        glfwSwapInterval(assets.glfwSwapInterval);
    }

    //state.setEnabled(GL_MULTISAMPLE, false);

    if (assets.useImGui) {
        m_editorInit = std::make_unique<FrameInit>(*m_window);
        m_editor = std::make_unique<editor::EditorFrame>(*m_window);

        PrepareContext ctx{ m_registry.get()};

        m_editorInit->prepare(ctx);
        m_editor->prepare(ctx);
    }

    if (false) {
        auto& engine = audio::AudioEngine::get();
        audio::sound_id soundId = engine.registerSound("audio/Stream Medium 01_8CC7FF9E_normal_mono.wav");

        audio::source_id sourceId = engine.registerSource(soundId);
        auto* source = engine.getSource(sourceId);
        if (source) {
            // TODO KI spatial left/right requires *MONO* sound
            source->m_pos = { 0.1f, 0.0f, 0.0f };
            source->update();
        }

        audio::listener_id listenerId = engine.registerListener();
        engine.setActiveListener(listenerId);

        engine.playSource(sourceId);
    }

    return 0;
}

int SampleApp::onUpdate(const ki::RenderClock& clock) {
    auto* scene = m_currentScene.get();
    if (!scene) return 0;

    {
        UpdateContext ctx(
            clock,
            m_currentScene->m_registry.get());

        scene->updateRT(ctx);
    }

    return 0;
}

int SampleApp::onPost(const ki::RenderClock& clock) {
    auto* scene = m_currentScene.get();
    if (!scene) return 0;

    {
        UpdateContext ctx(
            clock,
            m_currentScene->m_registry.get());

        scene->postRT(ctx);
    }

    return 0;
}

int SampleApp::onRender(const ki::RenderClock& clock)
{
    const auto& assets = Assets::get();

    auto* scene = m_currentScene.get();
    Window* window = m_window.get();

    if (!scene) return 0;

    Node* cameraNode = scene->getActiveCameraNode();
    if (!cameraNode) return 0;

    auto& state = kigl::GLState::get();
    const glm::uvec2& size = window->getSize();

    {
        UpdateViewContext ctx(
            clock,
            m_currentScene->m_registry.get(),
            size.x,
            size.y);

        scene->updateViewRT(ctx);
    }

    RenderContext ctx(
        "TOP",
        nullptr,
        clock,
        m_currentScene->m_registry.get(),
        m_currentScene->m_renderData.get(),
        m_currentScene->m_nodeDraw.get(),
        m_currentScene->m_batch.get(),
        &cameraNode->m_camera->getCamera(),
        assets.nearPlane,
        assets.farPlane,
        size.x,
        size.y,
        &m_debugContext);
    {
        ctx.m_forceWireframe = assets.forceWireframe;
        ctx.m_useLight = assets.useLight;

        if (m_debugContext.m_nodeDebugEnabled) {
            ctx.m_forceWireframe |= m_debugContext.m_forceWireframe;
        }

        // https://paroj.github.io/gltut/apas04.html
        if (assets.rasterizerDiscard) {
            //glEnable(GL_RASTERIZER_DISCARD);
            state.setEnabled(GL_RASTERIZER_DISCARD, true);
        }

        //m_state.useProgram(0);
        //m_state.bindVAO(0);

        state.setEnabled(GL_PROGRAM_POINT_SIZE, true);
        glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

        // make clear color by default black
        // => ensure "sane" start state for each loop
        state.clearColor(BLACK_COLOR);

        if (assets.useImGui) {
            m_editor->bind(ctx);
            state.clear();
        }

        scene->bind(ctx);
        scene->draw(ctx);
        scene->unbind(ctx);
    }

    {
        const auto* input = window->m_input.get();
        InputState state{
            input->isModifierDown(Modifier::CONTROL),
            input->isModifierDown(Modifier::SHIFT),
            input->isModifierDown(Modifier::ALT),
            glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT) != 0,
            glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) != 0,
        };


        if (state.mouseLeft != m_lastInputState.mouseLeft &&
            state.mouseLeft == GLFW_PRESS &&
            input->allowMouse())
        {
            if (state.ctrl)
            {
                selectNode(ctx, scene, state, m_lastInputState);
            }
            else if (state.shift)
            {
                raycastPlayer(ctx, scene, state, m_lastInputState);
            }
        }

        m_lastInputState = state;
    }

    if (assets.useImGui) {
        if (assets.imGuiDemo || m_editor->getState().m_imguiDemo) {
            ImGui::ShowDemoWindow();
        }

        m_editor->draw(ctx);
        m_editor->render(ctx);
    }

    frustumDebug(ctx, clock);

    return 0;
}

void SampleApp::frustumDebug(
    const RenderContext& ctx,
    const ki::RenderClock& clock)
{
    const auto& assets = ctx.m_assets;

    if (!assets.frustumDebug) return;

    auto* scene = m_currentScene.get();
    if (!scene) return;

    m_frustumElapsedSecs += clock.elapsedSecs;
    if (m_frustumElapsedSecs >= 10) {
        m_frustumElapsedSecs -= 10;

        auto counters = scene->getCounters(true);
        m_drawCount += counters.u_drawCount;
        m_skipCount += counters.u_skipCount;

        auto countersLocal = scene->getCountersLocal(true);

        if (assets.frustumCPU) {
            auto ratio = (float)countersLocal.u_skipCount / (float)countersLocal.u_drawCount;
            KI_INFO_OUT(fmt::format(
                "BATCH: cpu-draw={}, cpu-skip={}, cpu-ratio={}",
                countersLocal.u_drawCount, countersLocal.u_skipCount, ratio));
        }

        if (assets.frustumGPU) {
            auto ratio = (float)m_skipCount / (float)m_drawCount;
            auto frameDraw = (float)m_drawCount / (float)clock.frameCount;
            auto frameSkip = (float)m_skipCount / (float)clock.frameCount;

            KI_INFO_OUT(fmt::format(
                "{}: total-frames={}, gpu-draw={}, gpu-skip={}, gpu-ratio={}",
                ctx.m_name, clock.frameCount, m_drawCount, m_skipCount, ratio));

            KI_INFO(fmt::format(
                "{}: gpu-frame-draw={}, gpu-frame-skip={}",
                ctx.m_name, frameDraw, frameSkip));
        }
    }
}

void SampleApp::showFps(const ki::FpsCounter& fpsCounter)
{
    Engine::showFps(fpsCounter);

    auto fpsText = fmt::format("{} fps", round(fpsCounter.getAvgFps()));

    auto handle = pool::NodeHandle::toHandle(fpsNodeId);

    script::CommandEngine::get().addCommand(
        0,
        script::SetTextNode{
            handle,
            0.f,
            fpsText
        });
}

void SampleApp::onDestroy()
{
    KI_INFO_OUT("APP: destroy");

    *m_alive = false;

    if (!m_loaders.empty()) {
        KI_INFO_OUT("APP: stopping loaders...");
        for (auto& loader : m_loaders) {
            loader->destroy();
        }
        for (auto& loader : m_loaders) {
            // NOTE KI wait for worker threads to shutdown
            while (loader->isRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        KI_INFO_OUT("APP: loaders stopped!");
    }

    {
        m_sceneUpdater->destroy();
        m_particleUpdater->destroy();
        m_animationUpdater->destroy();
    }

    if (m_sceneUpdater) {
        KI_INFO_OUT("APP: stopping WT...");

        // NOTE KI wait for worker threads to shutdown
        while (m_sceneUpdater->isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        KI_INFO_OUT("APP: WT stopped!");
    }

    if (m_particleUpdater) {
        KI_INFO_OUT("APP: stopping PS...");

        // NOTE KI wait for worker threads to shutdown
        while (m_particleUpdater->isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        KI_INFO_OUT("APP: PS stopped!");
    }

    if (m_animationUpdater) {
        KI_INFO_OUT("APP: stopping AS...");

        // NOTE KI wait for worker threads to shutdown
        while (m_animationUpdater->isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        KI_INFO_OUT("APP: AS stopped!");
    }

    if (m_currentScene) {
        m_currentScene->destroy();
    }

    KI_INFO_OUT("APP: stopped all!");
}

void SampleApp::raycastPlayer(
    const RenderContext& ctx,
    Scene* scene,
    const InputState& inputState,
    const InputState& lastInputState)
{
    const Node* player = nullptr;

    if (!player)
    {
        player = pool::NodeHandle::toNode(SID("player"));
    }
    if (!player)
    {
        player = pool::NodeHandle::toNode(SID("player_2"));
    }
    if (!player)
    {
        player = pool::NodeHandle::toNode(SID("fps_player"));
    }
    if (!player)
    {
        player = pool::NodeHandle::toNode(SID("fps_player_2"));
    }

    if (player) {
        const auto& state = player->getState();
        const auto& hits = physics::PhysicsEngine::get().rayCast(
            state.getWorldPosition(),
            state.getViewFront(),
            util::as_integer(physics::Category::ray_player_fire),
            util::as_integer(physics::Category::npc),
            player->toHandle());

        if (!hits.empty()) {
            for (auto& hit : hits) {
                auto* node = hit.toNode();
                KI_INFO_OUT(fmt::format("HIT: {}", node->getName()));
            }
        }
    }
}

void SampleApp::selectNode(
    const RenderContext& ctx,
    Scene* scene,
    const InputState& inputState,
    const InputState& lastInputState)
{
    const auto& assets = ctx.m_assets;
    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;

    auto& debugContext = render::DebugContext::modify();

    const bool selectMode = inputState.ctrl;

    ki::node_id nodeId = scene->getObjectID(ctx, m_window->m_input->mouseX, m_window->m_input->mouseY);
    auto* node = pool::NodeHandle::toNode(nodeId);

    if (selectMode) {
        auto* volumeNode = pool::NodeHandle::toNode(assets.volumeId);

        // deselect
        if (node && node->isSelected()) {
            nodeRegistry.selectNode(pool::NodeHandle::NULL_HANDLE, false);

            if (volumeNode) {
                auto* controller = ControllerRegistry::get().get<VolumeController>(volumeNode);
                if (controller) {
                    controller->setTargetId(pool::NodeHandle::NULL_HANDLE);
                }
            }

            {
                event::Event evt { event::Type::audio_source_pause };
                evt.body.audioSource.id = node->m_audioSourceIds[0];
                ctx.m_registry->m_dispatcherWorker->send(evt);
            }

            return;
        }

        // select
        nodeRegistry.selectNode(node ? node->toHandle() : pool::NodeHandle::NULL_HANDLE, inputState.shift);

        if (volumeNode) {
            auto* controller = ControllerRegistry::get().get<VolumeController>(volumeNode);
            if (controller) {
                controller->setTargetId(node ? node->toHandle() : pool::NodeHandle::NULL_HANDLE);
            }
        }
        KI_INFO(fmt::format("selected: {}", nodeId));

        if (node) {
            if (m_debugContext.m_selectionAxis != glm::vec3{0.f}) {
                event::Event evt { event::Type::command_rotate };
                evt.body.command = {
                    .target = node->getId(),
                    .duration = 5,
                    .relative = true,
                    .data = m_debugContext.m_selectionAxis,
                    .data2 = { 360.f, 0, 0 },
                };
                ctx.m_registry->m_dispatcherWorker->send(evt);
            }

            {
                event::Event evt { event::Type::audio_source_play };
                evt.body.audioSource.id = node->m_audioSourceIds[0];
                ctx.m_registry->m_dispatcherWorker->send(evt);
            }

            m_editor->getState().m_selectedNode = node->toHandle();
        }
    }
    //else if (playerMode) {
    //    if (node && inputState.ctrl) {
    //        auto exists = ControllerRegistry::get().hasController(node);
    //        if (exists) {
    //            event::Event evt { event::Type::node_activate };
    //            evt.body.node.target = node->getId();
    //            ctx.m_registry->m_dispatcherWorker->send(evt);
    //        }

    //        node = nullptr;
    //    }
    //}
    //else if (cameraMode) {
    //    // NOTE KI null == default camera
    //    event::Event evt { event::Type::camera_activate };
    //    evt.body.node.target = node->getId();
    //    ctx.m_registry->m_dispatcherWorker->send(evt);

    //    node = nullptr;
    //}
}

Assets SampleApp::loadAssets()
{
    AssetsLoader loader{ "scene/assets.yml" };
    return loader.load();
}

std::shared_ptr<Scene> SampleApp::loadScene()
{
    const auto& assets = Assets::get();

    auto scene = std::make_shared<Scene>(m_registry, m_alive);

    {
        if (!assets.sceneFile.empty()) {
            loader::Context ctx{
                m_alive,
                m_asyncLoader,
                assets.sceneDir,
                assets.sceneFile,
            };
            std::unique_ptr<loader::SceneLoader> loader = std::make_unique<loader::SceneLoader>(ctx);
            m_loaders.push_back(std::move(loader));
        }
    }

    for (auto& loader : m_loaders) {
        KI_INFO_OUT(fmt::format("LOAD_SCENE: {}", loader->m_ctx.str()));
        loader->prepare(m_registry);
        loader->load();
    }

    //m_testSetup = std::make_unique<TestSceneSetup>(
    //    m_alive,
    //    m_asyncLoader);

    //m_testSetup->setup(
    //    scene->m_registry
    //);

    scene->prepareRT();

    return scene;
}
