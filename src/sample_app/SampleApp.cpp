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

#include "editor/EditorFrame.h"

#include "asset/DynamicCubeMap.h"
#include "material/Material.h"

#include "backend/gl/PerformanceCounters.h"

#include "script/CommandEngine.h"
#include "script/api/SetTextNode.h"
#include "script/api/Cancel.h"
#include "script/api/Wait.h"
#include "script/api/MoveNode.h"

#include "event/Dispatcher.h"

#include "audio/Source.h"
#include "audio/AudioEngine.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"

#include "engine/AssetsLoader.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"
#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/NodeDraw.h"

#include "loader/SceneLoader.h"

#include "scene/Scene.h"
#include "scene/SceneUpdater.h"
#include "scene/ParticleUpdater.h"
#include "scene/AnimationUpdater.h"

#include "TestSceneSetup.h"

#include "gui/Input.h"
#include "gui/Window.h"

#include "decal/DecalSystem.h"
#include "decal/DecalRegistry.h"

#include "physics/PhysicsEngine.h"
#include "physics/RayHit.h"
#include "physics/physics_util.h"

namespace {
    const glm::vec4 BLACK_COLOR{ 0.f };

    ki::node_id fpsNodeId = SID("fps_counter");

    std::vector<script::command_id> g_rayMarkers;

    constexpr float HIT_RATE = 0.25f;
    float g_hitElapsed = 0.f;
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
        auto& dbg = render::DebugContext::modify();

        dbg.m_glfwSwapInterval = assets.glfwSwapInterval;
        dbg.m_gBufferScale = assets.gBufferScale;

        dbg.m_layers = assets.layers;

        dbg.m_frustumEnabled = assets.frustumEnabled;
        dbg.m_forceLineMode = assets.forceLineMode;
        dbg.m_showNormals = assets.showNormals;
        dbg.m_shadowVisual = assets.shadowVisual;

        dbg.m_cubeMapEnabled = assets.cubeMapEnabled;

        dbg.m_showVolume = assets.showVolume;
        dbg.m_showSelectionVolume = assets.showSelectionVolume;
        dbg.m_showEnvironmentProbe = assets.showEnvironmentProbe;

        dbg.m_physicsUpdateEnabled = assets.physicsUpdateEnabled;
        dbg.m_physicsShowObjects = assets.physicsShowObjects;

        dbg.m_physics_dContactMu2 = assets.physics_dContactMu2;
        dbg.m_physics_dContactSlip1 = assets.physics_dContactSlip1;
        dbg.m_physics_dContactSlip2 = assets.physics_dContactSlip2;
        dbg.m_physics_dContactRolling = assets.physics_dContactRolling;
        dbg.m_physics_dContactBounce = assets.physics_dContactBounce;
        dbg.m_physics_dContactMotion1 = assets.physics_dContactMotion1;
        dbg.m_physics_dContactMotion2 = assets.physics_dContactMotion2;
        dbg.m_physics_dContactMotionN = assets.physics_dContactMotionN;
        dbg.m_physics_dContactSoftERP = assets.physics_dContactSoftERP;
        dbg.m_physics_dContactSoftCFM = assets.physics_dContactSoftCFM;
        dbg.m_physics_dContactApprox1 = assets.physics_dContactApprox1;
        dbg.m_physics_dContactFDir1 = assets.physics_dContactFDir1;

        dbg.m_physics_mu = assets.physics_mu;
        dbg.m_physics_mu2 = assets.physics_mu2;
        dbg.m_physics_rho = assets.physics_rho;
        dbg.m_physics_rho2 = assets.physics_rho2;
        dbg.m_physics_rhoN = assets.physics_rhoN;
        dbg.m_physics_slip1 = assets.physics_slip1;
        dbg.m_physics_slip2 = assets.physics_slip2;
        dbg.m_physics_bounce = assets.physics_bounce;
        dbg.m_physics_bounce_vel = assets.physics_bounce_vel;
        dbg.m_physics_motion1 = assets.physics_motion1;
        dbg.m_physics_motion2 = assets.physics_motion2;
        dbg.m_physics_motionN = assets.physics_motionN;
        dbg.m_physics_soft_erp = assets.physics_soft_erp;
        dbg.m_physics_soft_cfm = assets.physics_soft_cfm;

        dbg.m_parallaxEnabled = assets.parallaxEnabled;
        dbg.m_parallaxMethod = assets.parallaxMethod;
        dbg.m_parallaxDebugEnabled = assets.parallaxDebugEnabled;
        dbg.m_parallaxDebugDepth = assets.parallaxDebugDepth;

        dbg.m_decalId = SID("blood_1");

        dbg.m_drawDebug = assets.drawDebug;

        dbg.m_prepassDepthEnabled = assets.prepassDepthEnabled;
        dbg.m_effectOitEnabled = assets.effectOitEnabled;
        dbg.m_effectEmissionEnabled = assets.effectEmissionEnabled;
        dbg.m_effectFogEnabled = assets.effectFogEnabled;

        dbg.m_fogColor = assets.fogColor;
        dbg.m_fogStart = assets.fogStart;
        dbg.m_fogEnd = assets.fogEnd;
        dbg.m_fogDensity = assets.fogDensity;

        dbg.m_hdrGamma = assets.hdrGamma;
        dbg.m_hdrExposure = assets.hdrExposure;

        dbg.m_effectBloomEnabled = assets.effectBloomEnabled;
        dbg.m_effectBloomIterations = assets.effectBloomIterations;

        dbg.m_particleEnabled = assets.particleEnabled;
        dbg.m_decalEnabled = assets.decalEnabled;
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

    //state.setEnabled(GL_MULTISAMPLE, false);

    if (assets.useImGui) {
        m_editorInit = std::make_unique<FrameInit>(*m_window);
        m_editor = std::make_unique<editor::EditorFrame>(*m_window);

        PrepareContext ctx{ m_registry.get()};

        m_editorInit->prepare(ctx);
        m_editor->prepare(ctx);
    }

    //if (false) {
    //    auto& engine = audio::AudioEngine::get();
    //    audio::sound_id soundId = engine.registerSound("audio/Stream Medium 01_8CC7FF9E_normal_mono.wav");

    //    audio::source_id sourceId = engine.registerSource(soundId);
    //    auto* source = engine.getSource(sourceId);
    //    if (source) {
    //        // TODO KI spatial left/right requires *MONO* sound
    //        source->m_pos = { 0.1f, 0.0f, 0.0f };
    //        source->update();
    //    }

    //    audio::listener_id listenerId = engine.registerListener();
    //    engine.setActiveListener(listenerId);

    //    engine.playSource(sourceId);
    //}

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

    {
        auto& dbg = render::DebugContext::modify();

        glfwSwapInterval(dbg.m_glfwSwapInterval);
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
        &m_currentScene->m_nodeDraw.get()->m_collection,
        m_currentScene->m_renderData.get(),
        m_currentScene->m_nodeDraw.get(),
        m_currentScene->m_batch.get(),
        &cameraNode->m_camera->getCamera(),
        assets.nearPlane,
        assets.farPlane,
        size.x,
        size.y,
        &m_dbg);
    {
        if (const auto* layer = LayerInfo::findLayer(LAYER_MAIN); layer) {
            ctx.m_layer = layer->m_index;
        }
        ctx.m_forceLineMode = assets.forceLineMode;
        ctx.m_useLight = assets.useLight;

        if (m_dbg.m_nodeDebugEnabled) {
            ctx.m_forceLineMode |= m_dbg.m_forceLineMode;
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
            //else if (state.shift)
            //{
            //    shoot(ctx, scene, state, m_lastInputState);
            //}
        }

        if (state.mouseRight == GLFW_PRESS &&
            input->allowMouse())
        {
            if (state.ctrl)
            {
                shoot(ctx, scene, state, m_lastInputState);
            }
        }

        m_lastInputState = state;
    }

    if (assets.useImGui) {
        ctx.m_state.bindFrameBuffer(0, false);

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
    auto* player = m_currentScene->getActiveNode();
    if (!player) return;

    {
        const auto* snapshot = player->getSnapshotRT();
        if (!snapshot) return;

        const auto& hits = physics::PhysicsEngine::get().rayCast(
            snapshot->getWorldPosition(),
            snapshot->getViewFront(),
            100.f,
            physics::mask(physics::Category::ray_player_fire),
            physics::mask(physics::Category::npc),
            player->toHandle(),
            true);

        if (!hits.empty()) {
            for (auto& hit : hits) {
                auto* node = hit.handle.toNode();
                if (!node) continue;

                KI_INFO_OUT(fmt::format(
                    "PLAYER_HIT: node={}, pos={}, normal={}, depth={}",
                    node ? node->getName() : "N/A",
                    hit.pos,
                    hit.normal,
                    hit.depth));
            }
        }
    }

    {
        glm::vec2 screenPos{ m_window->m_input->mouseX, m_window->m_input->mouseY };

        const auto startPos = ctx.unproject(screenPos, .01f);
        const auto endPos = ctx.unproject(screenPos, .8f);
        const auto dir = glm::normalize(endPos - startPos);

        KI_INFO_OUT(fmt::format(
            "UNPROJECT: screenPos={}, z0={}, z1={}",
            screenPos, startPos, endPos));

        auto greenBall = pool::NodeHandle::toNode(SID("green_ball"));
        auto redBall = pool::NodeHandle::toNode(SID("red_ball"));

        for (auto& cmdId : g_rayMarkers) {
            script::CommandEngine::get().cancelCommand(cmdId);
        }
        g_rayMarkers.clear();

        if (greenBall) {
            auto cmdId = 0;
            for (int i = 0; i < 5; i++) {
                cmdId = script::CommandEngine::get().addCommand(
                    cmdId,
                    script::MoveNode{
                        greenBall->toHandle(),
                        0.f,
                        false,
                        startPos
                    });
                g_rayMarkers.push_back(cmdId);

                if (i == 0) {
                    cmdId = script::CommandEngine::get().addCommand(
                        cmdId,
                        script::Wait{
                            2.f
                        });
                    g_rayMarkers.push_back(cmdId);
                }

                cmdId = script::CommandEngine::get().addCommand(
                    cmdId,
                    script::MoveNode{
                        greenBall->toHandle(),
                        2.f,
                        true,
                        dir * 5.f
                    });
                g_rayMarkers.push_back(cmdId);
            }
            {
                cmdId = script::CommandEngine::get().addCommand(
                    cmdId,
                    script::MoveNode{
                        greenBall->toHandle(),
                        0.f,
                        false,
                        startPos
                    });
                g_rayMarkers.push_back(cmdId);
            }
        }
        if (redBall) {
            auto cmdId = script::CommandEngine::get().addCommand(
                0,
                script::MoveNode{
                    redBall->toHandle(),
                    0.f,
                    false,
                    endPos
                });
            g_rayMarkers.push_back(cmdId);
        }

        const auto& hits = physics::PhysicsEngine::get().rayCast(
            startPos,
            dir,
            400.f,
            physics::mask(physics::Category::ray_player_fire),
            physics::mask(physics::Category::npc, physics::Category::prop),
            //physics::mask(physics::Category::all),
            player->toHandle(),
            true);
    }
}

void SampleApp::shoot(
    const RenderContext& ctx,
    Scene* scene,
    const InputState& inputState,
    const InputState& lastInputState)
{
    auto* player = m_currentScene->getActiveNode();
    if (!player) return;

    {
        const auto& dbg = render::DebugContext::get();

        glm::vec2 screenPos{ m_window->m_input->mouseX, m_window->m_input->mouseY };

        const auto startPos = ctx.unproject(screenPos, .01f);
        const auto endPos = ctx.unproject(screenPos, .8f);
        const auto dir = glm::normalize(endPos - startPos);

        const auto& hits = physics::PhysicsEngine::get().rayCast(
            startPos,
            dir,
            400.f,
            physics::mask(physics::Category::ray_player_fire),
            physics::mask(physics::Category::npc, physics::Category::prop),
            //physics::mask(physics::Category::all),
            player->toHandle(),
            true);

        g_hitElapsed += ctx.m_clock.elapsedSecs;

        if (!hits.empty() && g_hitElapsed >= HIT_RATE) {
            g_hitElapsed -= HIT_RATE;

            for (auto& hit : hits) {
                auto* node = hit.handle.toNode();
                //KI_INFO_OUT(fmt::format(
                //    "SCREEN_HIT: node={}, pos={}, normal={}, depth={}",
                //    node ? node->getName() : "N/A",
                //    hit.pos,
                //    hit.normal,
                //    hit.depth));

                auto sid = dbg.m_decalId;
                auto df = decal::DecalRegistry::get().getDecal(sid);
                KI_INFO_OUT(fmt::format("DECAL: name={}, valid={}", sid.str(), df ? true : false));

                auto decal = df.createForHit(ctx, hit.handle, hit.pos, glm::normalize(hit.normal));

                decal::DecalSystem::get().addDecal(decal);

                //KI_INFO_OUT(fmt::format(
                //    "DECAL: node={}, pos={}, normal={}",
                //    node ? node->getName() : "N/A",
                //    decal.m_position,
                //    decal.m_normal));
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

    auto& dbg = render::DebugContext::modify();

    const bool selectMode = inputState.ctrl;

    ki::node_id nodeId = scene->getObjectID(ctx, m_window->m_input->mouseX, m_window->m_input->mouseY);
    auto* node = pool::NodeHandle::toNode(nodeId);

    if (selectMode) {
        // deselect
        if (node && node->isSelected()) {
            nodeRegistry.selectNode(pool::NodeHandle::NULL_HANDLE, false);

            //{
            //    event::Event evt { event::Type::audio_source_pause };
            //    evt.body.audioSource.id = node->m_audioSourceIds[0];
            //    ctx.m_registry->m_dispatcherWorker->send(evt);
            //}

            return;
        }

        // select
        nodeRegistry.selectNode(node ? node->toHandle() : pool::NodeHandle::NULL_HANDLE, inputState.shift);

        KI_INFO(fmt::format("selected: {}", nodeId));

        if (node) {
            if (m_dbg.m_selectionAxis != glm::vec3{0.f}) {
                event::Event evt { event::Type::command_rotate };
                evt.body.command = {
                    .target = node->getId(),
                    .duration = 5,
                    .relative = true,
                    .data = m_dbg.m_selectionAxis,
                    .data2 = { 360.f, 0, 0 },
                };
                ctx.m_registry->m_dispatcherWorker->send(evt);
            }

            //{
            //    event::Event evt { event::Type::audio_source_play };
            //    evt.body.audioSource.id = node->m_audioSourceIds[0];
            //    ctx.m_registry->m_dispatcherWorker->send(evt);
            //}

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
