#include "SampleApp.h"

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "kigl/kigl.h"

#include "editor/EditorFrame.h"

#include "asset/DynamicCubeMap.h"

#include "backend/gl/PerformanceCounters.h"

#include "controller/VolumeController.h"

#include "event/Dispatcher.h"

#include "audio/Source.h"
#include "audio/AudioEngine.h"

#include "mesh/MeshType.h"

#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"

#include "engine/AssetsFile.h"

#include "engine/UpdateContext.h"
#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"

#include "loader/SceneLoader.h"

#include "scene/Scene.h"
#include "scene/SceneUpdater.h"

#include "TestSceneSetup.h"

#include "gui/Input.h"
#include "gui/Window.h"


namespace {
    const glm::vec4 BLACK_COLOR{ 0.f };
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

    m_assets = loadAssets();
    return 0;
}

int SampleApp::onSetup() {
    m_currentScene = loadScene();
    m_sceneUpdater = std::make_shared<SceneUpdater>(
        m_assets,
        m_registry,
        m_alive);

    m_sceneUpdater->start();

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (m_assets.glfwSwapInterval >= 0) {
        glfwSwapInterval(m_assets.glfwSwapInterval);
    }

    //state.setEnabled(GL_MULTISAMPLE, false);

    if (m_assets.useIMGUI) {
        m_frameInit = std::make_unique<FrameInit>(*m_window);
        m_frame = std::make_unique<EditorFrame>(*m_window);
    }

    if (false) {
        auto engine = m_registry->m_audioEngine;
        audio::sound_id soundId = engine->registerSound("audio/Stream Medium 01_8CC7FF9E_normal_mono.wav");

        audio::source_id sourceId = engine->registerSource(soundId);
        auto* source = engine->getSource(sourceId);
        if (source) {
            // TODO KI spatial left/right requires *MONO* sound
            source->m_pos = { 0.1f, 0.0f, 0.0f };
            source->update();
        }

        audio::listener_id listenerId = engine->registerListener();
        engine->setActiveListener(listenerId);

        engine->playSource(sourceId);
    }

    return 0;
}

int SampleApp::onUpdate(const ki::RenderClock& clock) {
    auto* updater = m_currentScene.get();
    if (!updater) return 0;

    {
        UpdateContext ctx(
            clock,
            m_assets,
            m_currentScene->m_registry.get());

        updater->updateRT(ctx);
    }

    return 0;
}

int SampleApp::onPost(const ki::RenderClock& clock) {
    auto* updater = m_currentScene.get();
    if (!updater) return 0;

    {
        UpdateContext ctx(
            clock,
            m_assets,
            m_currentScene->m_registry.get());

        updater->postRT(ctx);
    }

    return 0;
}

int SampleApp::onRender(const ki::RenderClock& clock) {
    auto* scene = m_currentScene.get();
    Window* window = m_window.get();

    if (!scene) return 0;

    Node* cameraNode = scene->getActiveCameraNode();
    if (!cameraNode) return 0;


    const glm::uvec2& size = window->getSize();

    {
        UpdateViewContext ctx(
            clock,
            m_assets,
            m_currentScene->m_registry.get(),
            size.x,
            size.y);

        scene->updateViewRT(ctx);
    }

    RenderContext ctx(
        "TOP",
        nullptr,
        clock,
        m_assets,
        m_currentScene->m_registry.get(),
        m_currentScene->m_renderData.get(),
        m_currentScene->m_nodeDraw.get(),
        m_currentScene->m_batch.get(),
        m_state,
        cameraNode->m_camera.get(),
        m_assets.nearPlane,
        m_assets.farPlane,
        size.x, size.y);
    {
        ctx.m_forceWireframe = m_assets.forceWireframe;
        ctx.m_useLight = m_assets.useLight;

        // https://paroj.github.io/gltut/apas04.html
        if (m_assets.rasterizerDiscard) {
            //glEnable(GL_RASTERIZER_DISCARD);
            ctx.m_state.setEnabled(GL_RASTERIZER_DISCARD, true);
        }

        //m_state.useProgram(0);
        //m_state.bindVAO(0);

        m_state.setEnabled(GL_PROGRAM_POINT_SIZE, true);
        glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

        // make clear color by default black
        // => ensure "sane" start state for each loop
        ctx.m_state.clearColor(BLACK_COLOR);

        if (m_assets.useIMGUI) {
            m_frame->bind(ctx);
            m_state.clear();
        }

        scene->bind(ctx);
        scene->draw(ctx);
        scene->unbind(ctx);
    }

    {
        InputState state{
            window->m_input->isModifierDown(Modifier::CONTROL),
            window->m_input->isModifierDown(Modifier::SHIFT),
            window->m_input->isModifierDown(Modifier::ALT),
            glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT) != 0,
            glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) != 0,
        };

        if (state.mouseLeft != m_lastInputState.mouseLeft &&
            state.mouseLeft == GLFW_PRESS &&
            state.ctrl)
        {
            if ((state.shift || state.ctrl || state.alt) && (!m_assets.useIMGUI || !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))) {
                selectNode(ctx, scene, state, m_lastInputState);
            }
        }

        m_lastInputState = state;
    }

    if (m_assets.useIMGUI) {
        ImGui::ShowDemoWindow();

        m_frame->draw(ctx);
        m_frame->render(ctx);
    }

    frustumDebug(ctx, clock);

    return 0;
}

void SampleApp::frustumDebug(
    const RenderContext& ctx,
    const ki::RenderClock& clock)
{
    if (!m_assets.frustumDebug) return;

    auto* scene = m_currentScene.get();
    if (!scene) return;

    m_frustumElapsedSecs += clock.elapsedSecs;
    if (m_frustumElapsedSecs >= 10) {
        m_frustumElapsedSecs -= 10;

        auto counters = scene->getCounters(true);
        m_drawCount += counters.u_drawCount;
        m_skipCount += counters.u_skipCount;

        auto countersLocal = scene->getCountersLocal(true);

        if (m_assets.frustumCPU) {
            auto ratio = (float)countersLocal.u_skipCount / (float)countersLocal.u_drawCount;
            KI_INFO_OUT(fmt::format(
                "BATCH: cpu-draw={}, cpu-skip={}, cpu-ratio={}",
                countersLocal.u_drawCount, countersLocal.u_skipCount, ratio));
        }

        if (m_assets.frustumGPU) {
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

    if (m_sceneUpdater) {
        KI_INFO_OUT("APP: stopping worker...");
        m_sceneUpdater->destroy();

        // NOTE KI wait for worker threads to shutdown
        while (m_sceneUpdater->isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        KI_INFO_OUT("APP: worker stopped!");
    }

    if (m_currentScene) {
        m_currentScene->destroy();
    }

    KI_INFO_OUT("APP: stopped all!");
}

void SampleApp::selectNode(
    const RenderContext& ctx,
    Scene* scene,
    const InputState& inputState,
    const InputState& lastInputState)
{
    const bool cameraMode = inputState.ctrl && inputState.alt && inputState.shift;
    const bool playerMode = inputState.ctrl && inputState.alt && !cameraMode;
    const bool selectMode = inputState.ctrl && !playerMode && !cameraMode;

    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;
    ki::node_id nodeId = scene->getObjectID(ctx, m_window->m_input->mouseX, m_window->m_input->mouseY);
    auto* node = nodeRegistry.getNode(nodeId);

    if (selectMode) {
        auto* volumeNode = nodeRegistry.getNode(ctx.m_assets.volumeUUID);

        // deselect
        if (node && node->isSelected()) {
            nodeRegistry.selectNodeById(-1, false);

            if (volumeNode) {
                auto* controller = ctx.m_registry->m_controllerRegistry->get<VolumeController>(volumeNode);
                if (controller) {
                    controller->setTarget(-1);
                }
            }

            {
                event::Event evt { event::Type::audio_source_pause };
                evt.body.audioSource.id = node->m_audioSourceIds[0];
                ctx.m_registry->m_dispatcher->send(evt);
            }

            return;
        }

        // select
        nodeRegistry.selectNodeById(nodeId, inputState.shift);

        if (volumeNode) {
            auto* controller = ctx.m_registry->m_controllerRegistry->get<VolumeController>(volumeNode);
            if (controller) {
                controller->setTarget(node ? node->m_id : -1);
            }
        }
        KI_INFO(fmt::format("selected: {}", nodeId));

        if (node) {
            {
                event::Event evt { event::Type::animate_rotate };
                evt.body.animate = {
                    .target = node->m_id,
                    .duration = 5,
                    .relative = true,
                    .data = { 0.f, 1.f, 0.f },
                    .data2 = { 360.f, 0, 0 },
                };
                ctx.m_registry->m_dispatcher->send(evt);
            }

            {
                event::Event evt { event::Type::audio_source_play };
                evt.body.audioSource.id = node->m_audioSourceIds[0];
                ctx.m_registry->m_dispatcher->send(evt);
            }
        }
    } else if (playerMode) {
        if (node && inputState.ctrl) {
            auto exists = m_registry->m_controllerRegistry->hasController(node);
            if (exists) {
                event::Event evt { event::Type::node_activate };
                evt.body.node.target = node;
                ctx.m_registry->m_dispatcher->send(evt);
            }

            node = nullptr;
        }
    } else if (cameraMode) {
        // NOTE KI null == default camera
        event::Event evt { event::Type::camera_activate };
        evt.body.node.target = node;
        ctx.m_registry->m_dispatcher->send(evt);

        node = nullptr;
    }
}

Assets SampleApp::loadAssets()
{
    AssetsFile file{ "scene/assets.yml" };
    return file.load();
}

std::shared_ptr<Scene> SampleApp::loadScene()
{
    auto scene = std::make_shared<Scene>(m_assets, m_registry, m_alive);

    {
        if (!m_assets.sceneFile.empty()) {
            loader::Context ctx{
                m_assets,
                m_alive,
                m_asyncLoader,
                m_assets.sceneDir,
                m_assets.sceneFile,
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
    //    m_assets,
    //    m_alive,
    //    m_asyncLoader);

    //m_testSetup->setup(
    //    scene->m_registry
    //);

    scene->prepareRT();

    return scene;
}
