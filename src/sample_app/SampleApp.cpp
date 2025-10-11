#include "SampleApp.h"

#include <numbers>

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "util/glm_format.h"
#include "util/util.h"

#include "kigl/kigl.h"

#include "ki/sid.h"

#include "pool/NodeHandle.h"

#include "engine/InputContext.h"

#include "editor/EditorFrame.h"

#include "asset/DynamicCubeMap.h"
#include "material/Material.h"

#include "backend/gl/PerformanceCounters.h"

#include "script/CommandEngine.h"
#include "script/command/AudioPlay.h"
#include "script/command/AudioPause.h"
#include "script/command/AudioStop.h"
#include "script/command/SetTextNode.h"
#include "script/command/Cancel.h"
#include "script/command/Wait.h"
#include "script/command/MoveNode.h"
#include "script/command/SelectNode.h"
#include "script/command/RayCast.h"

#include "script/ScriptSystem.h"

#include "event/Dispatcher.h"

#include "mesh/LodMesh.h"

#include "model/NodeType.h"
#include "component/CameraComponent.h"

#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"
#include "registry/ControllerRegistry.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"
#include "engine/UpdateViewContext.h"

#include "nav/NavigationSystem.h"

#include "render/RenderContext.h"
#include "render/NodeDraw.h"
#include "render/WindowBuffer.h"

#include "loader/Context.h"
#include "loader/SceneLoader.h"

#include "shader/ProgramRegistry.h"

#include "scene/Scene.h"
#include "scene/SceneUpdater.h"
#include "scene/ParticleUpdater.h"
#include "scene/AnimationUpdater.h"

#include "TestSceneSetup.h"

#include "gui/Input.h"
#include "gui/Window.h"
#include "gui/FrameContext.h"

#include "decal/DecalSystem.h"
#include "decal/DecalRegistry.h"

#include "physics/PhysicsSystem.h"
#include "physics/RayHit.h"
#include "physics/physics_util.h"

#include "action/ActionContext.h"
#include "action/RayCastPlayer.h"

namespace {
    const glm::vec4 BLACK_COLOR{ 0.f };

    ki::node_id fpsNodeId1 = SID("fps_counter");
    ki::node_id fpsNodeId2 = SID("prefab_fps_counter");

    render::Camera DEFAULT_CAMERA{};
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

    m_dbg.prepare();

    return 0;
}

int SampleApp::onSetup()
{
    const auto& assets = Assets::get();

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //state.setEnabled(GL_MULTISAMPLE, false);

    if (assets.useEditor) {
        m_editorFrameInit = std::make_shared<FrameInit>(m_window);
        m_editorFrame = std::make_unique<editor::EditorFrame>(m_window);

        PrepareContext ctx{ *this };

        m_editorFrameInit->prepare(ctx);
        m_editorFrame->prepare(ctx);

        m_listen_action_editor_scene_load.listen(
            event::Type::action_editor_scene_load,
            m_registry->m_dispatcherView,
            [this](const event::Event& e) {
                if (!e.attachment) return;
                const auto& filePath = e.attachment->pathEntry.filePath;
                onLoadScene(filePath);
			});

        m_listen_action_editor_scene_unload.listen(
            event::Type::action_editor_scene_unload,
            m_registry->m_dispatcherView,
            [this](const event::Event& e) {
                onUnloadScene();
            });
    }

    m_listen_scene_loaded.listen(
        event::Type::scene_loaded,
        m_registry->m_dispatcherView,
        [this](const event::Event& e) {
            stopLoader();
        });

    m_listen_scene_unload.listen(
        event::Type::scene_unload,
        m_registry->m_dispatcherView,
        [this](const event::Event& e) {
            stopLoader();
        });

    //m_currentScene = loadScene();

    m_registry->clear();

    return 0;
}

int SampleApp::onUpdate(const UpdateContext& ctx)
{
	const auto& assets = ctx.getAssets();
	const auto& dbg = ctx.getDebug();

    auto* scene = m_currentScene.get();

    if (scene)
	{
        scene->updateRT(ctx);
    }
    else {
        getRegistry()->m_dispatcherView->dispatchEvents();
    }

	{
        auto& dbg = debug::DebugContext::get();

        glfwSwapInterval(dbg.m_glfwSwapInterval);
    }

    return 0;
}

int SampleApp::onPost(const UpdateContext& ctx)
{
    if (auto* scene = m_currentScene.get(); scene)
	{
        scene->postRT(ctx);
    }

    return 0;
}

int SampleApp::onRender(const ki::RenderClock& clock)
{
    const auto& assets = Assets::get();

    auto* scene = m_currentScene.get();
    Window* window = m_window.get();

    auto& state = kigl::GLState::get();
    const glm::ivec2& size = window->getSize();

    if (scene) {
        UpdateViewContext ctx{
            *this,
            size.x,
            size.y };

        scene->updateViewRT(ctx);
    }

    render::Camera* camera = &DEFAULT_CAMERA;
    
    if (scene) {
        auto* cameraNode = scene->getActiveCameraNode();
        if (cameraNode) {
            camera = &cameraNode->m_camera->getCamera();
        }
    }

    if (scene)
	{
        render::RenderContext ctx(
            "TOP",
            nullptr,
            clock,
            m_registry.get(),
            scene->getCollection(),
            getRenderData(),
            getBatch(),
            camera,
            assets.nearPlane,
            assets.farPlane,
            size.x,
            size.y,
            m_dbg);

        if (const auto* layer = LayerInfo::findLayer(LAYER_MAIN); layer) {
            ctx.m_layer = layer->m_index;
        }
        //ctx.m_forceLineMode = assets.forceLineMode;
        ctx.m_forceLineMode = m_dbg.m_forceLineMode;

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
        state.setClearColor(BLACK_COLOR);

        if (scene) {
            scene->bind(ctx);
            scene->render(ctx);
            scene->unbind(ctx);
        }
    }
    else {
        render::RenderContext ctx(
            "TOP",
            nullptr,
            clock,
            m_registry.get(),
            nullptr,
            getRenderData(),
            getBatch(),
            camera,
            assets.nearPlane,
            assets.farPlane,
            size.x,
            size.y,
            m_dbg);

        auto* buffer = m_windowBuffer.get();

        buffer->bind(ctx);
        buffer->clear(
            ctx,
            GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
            { 0.f, 0.f, 0.f, 1.f });
    }

    processInput();

    if (assets.useEditor) {
        gui::FrameContext ctx{ *this };
        m_editorFrame->bind(ctx);
        m_editorFrame->draw(ctx);
        {
            InputContext ctx{ *this, *m_window->m_input };
            m_editorFrame->processInputs(ctx);
        }
        m_editorFrame->render(ctx);
        state.invalidateAll();
    }

    frustumDebug(clock);

    return 0;
}

void SampleApp::processInput()
{
    const auto& assets = Assets::get();
    auto* scene = m_currentScene.get();

    InputContext ctx{ *this, *m_window->m_input };
    const auto& inputState = ctx.getInputState();

    if (scene) {
        if (inputState.mouseRight == GLFW_PRESS &&
            ctx.getInput().allowMouse())
        {
            if (inputState.ctrl)
            {
                event::Event evt{ event::Type::action_game_shoot };
                getRegistry()->m_dispatcherView->send(evt);
            }
        }
    }
}

void SampleApp::frustumDebug(
    const ki::RenderClock& clock)
{
    const auto& assets = Assets::get();

    if (!assets.frustumDebug) return;

    auto* scene = m_currentScene.get();
    if (!scene) return;

    m_frustumElapsedSecs += clock.elapsedSecs;
    if (m_frustumElapsedSecs >= 10) {
        m_frustumElapsedSecs -= 10;

        auto counters = getCounters(true);
        m_drawCount += counters.u_drawCount;
        m_skipCount += counters.u_skipCount;

        auto countersLocal = getCountersLocal(true);

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
                "STOP: total-frames={}, gpu-draw={}, gpu-skip={}, gpu-ratio={}",
                clock.frameCount, m_drawCount, m_skipCount, ratio));

            KI_INFO(fmt::format(
                "TOP: gpu-frame-draw={}, gpu-frame-skip={}",
                frameDraw, frameSkip));
        }
    }
}

void SampleApp::showFps(const ki::FpsCounter& fpsCounter)
{
    Engine::showFps(fpsCounter);

    auto fpsText = fmt::format("{} fps", round(fpsCounter.getAvgFps()));

    auto handle = pool::NodeHandle::toHandle(fpsNodeId1);
    if (!handle) {
        handle = pool::NodeHandle::toHandle(fpsNodeId2);
    }

    script::CommandEngine::get().addCommand(
        0,
        script::SetTextNode{
            handle,
            fpsText
        });
}

void SampleApp::stopUpdaters()
{
    nav::NavigationSystem::get().stop();

    {
        if (m_sceneUpdater)
        {
            m_sceneUpdater->destroy();
        }
        if (m_particleUpdater)
        {
            m_particleUpdater->destroy();
        }
        if (m_animationUpdater)
        {
            m_animationUpdater->destroy();
        }
    }

    if (m_sceneUpdater) {
        KI_INFO_OUT("APP: stopping WT...");

        // NOTE KI wait for worker threads to shutdown
        while (m_sceneUpdater->isRunning()) {
            util::sleep(100);
        }
        KI_INFO_OUT("APP: WT stopped!");
        m_sceneUpdater = nullptr;
    }

    if (m_particleUpdater) {
        KI_INFO_OUT("APP: stopping PS...");

        // NOTE KI wait for worker threads to shutdown
        while (m_particleUpdater->isRunning()) {
            util::sleep(100);
        }
        KI_INFO_OUT("APP: PS stopped!");
        m_particleUpdater = nullptr;
    }

    if (m_animationUpdater) {
        KI_INFO_OUT("APP: stopping AS...");

        // NOTE KI wait for worker threads to shutdown
        while (m_animationUpdater->isRunning()) {
            util::sleep(100);
        }
        KI_INFO_OUT("APP: AS stopped!");
        m_animationUpdater = nullptr;
    }
}

void SampleApp::onDestroy()
{
    KI_INFO_OUT("APP: destroy");

    *m_alive = false;

    stopLoader();
    stopUpdaters();

    if (m_currentScene) {
        m_currentScene->destroy();
    }

    Engine::onDestroy();

    KI_INFO_OUT("APP: stopped all!");
}

void SampleApp::onLoadScene(const std::string& filePath)
{
    m_currentScene = loadScene(filePath);
}

void SampleApp::onUnloadScene()
{
    unloadScene();
}

void SampleApp::onShoot()
{
    const auto& input = *m_window->m_input;

    //shoot(ctx, m_currentScene.get(), input, inputState, m_lastInputState);
}

std::shared_ptr<Scene> SampleApp::loadScene(
    const std::string& filePath)
{
    const auto& assets = Assets::get();

    unloadScene();

    auto scene = std::make_shared<Scene>(*this);

    {
        if (!assets.sceneFile.empty()) {
            scene->setName(assets.sceneFile);

            auto ctx = std::make_shared<loader::Context>(
                m_asyncLoader,
                assets.sceneDir,
                filePath
            );
            scene->setFilePath(ctx->m_fullPath);
            scene->setName(ctx->m_name);

            m_loader = std::make_unique<loader::SceneLoader>(ctx);
        }
    }

    if (m_loader) {
        m_sceneUpdater = std::make_shared<SceneUpdater>(
            *this);

        m_particleUpdater = std::make_shared<ParticleUpdater>(
            *this);

        m_animationUpdater = std::make_shared<AnimationUpdater>(
            *this);

        m_sceneUpdater->start();
        m_particleUpdater->start();
        m_animationUpdater->start();
    }

    if (m_loader) {
        scene->prepareRT();
        ProgramRegistry::get().updateRT({ *this });
    }

    if (m_loader) {
        nav::NavigationSystem::get().start();
        script::ScriptSystem::get().start();
    }

    if (m_loader) {
        KI_INFO_OUT(fmt::format("LOAD_SCENE: {}", m_loader->m_ctx->str()));
        m_loader->prepare(getRegistry());
        m_loader->load();
    }

    return scene;
}

void SampleApp::unloadScene()
{
    stopLoader();
    stopUpdaters();

    if (!m_currentScene) return;
    m_currentScene->destroy();
    m_registry->clear();
    m_currentScene = nullptr;
}

void SampleApp::stopLoader()
{
    if (!m_loader) return;

    KI_INFO_OUT("APP: stopping loader...");
    m_loader->destroy();

    // NOTE KI wait for worker threads to shutdown
    while (m_loader->isRunning()) {
        util::sleep(100);
    }

    m_loader = nullptr;

    KI_INFO_OUT("APP: loader stopped!");
}
