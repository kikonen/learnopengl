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
#include "script/command/AudioPlay.h"
#include "script/command/AudioPause.h"
#include "script/command/AudioStop.h"
#include "script/command/SetTextNode.h"
#include "script/command/Cancel.h"
#include "script/command/Wait.h"
#include "script/command/MoveNode.h"
#include "script/command/SelectNode.h"
#include "script/command/RayCast.h"

#include "event/Dispatcher.h"

#include "mesh/LodMesh.h"

#include "model/NodeType.h"
#include "component/CameraComponent.h"

#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"
#include "registry/ControllerRegistry.h"

#include "engine/AssetsLoader.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"
#include "engine/UpdateViewContext.h"

#include "render/RenderContext.h"
#include "render/NodeDraw.h"

#include "loader/Context.h"
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

#include "physics/PhysicsSystem.h"
#include "physics/RayHit.h"
#include "physics/physics_util.h"

namespace {
    const glm::vec4 BLACK_COLOR{ 0.f };

    ki::node_id fpsNodeId1 = SID("fps_counter");
    ki::node_id fpsNodeId2 = SID("prefab_fps_counter");

    std::vector<script::command_id> g_rayMarkers;

    constexpr float HIT_RATE = 0.01f;
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
        auto& dbg = debug::DebugContext::modify();

        dbg.m_glfwSwapInterval = assets.glfwSwapInterval;
        dbg.m_gBufferScale = assets.gBufferScale;

        dbg.m_layers = assets.layers;

        dbg.m_frustumEnabled = assets.frustumEnabled;
        dbg.m_forceLineMode = assets.forceLineMode;
        dbg.m_showNormals = assets.showNormals;
        dbg.m_shadowVisual = assets.shadowVisual;

        dbg.m_lightEnabled = assets.lightEnabled;

        dbg.m_skyboxEnabled = assets.skyboxEnabled;
        dbg.m_skyboxColorEnabled = assets.skyboxColorEnabled;
        dbg.m_skyboxColor = assets.skyboxColor;

        dbg.m_mirrorMapEnabled = assets.mirrorMapEnabled;
        dbg.m_mirrorMapReflectionBufferScale = assets.mirrorMapReflectionBufferScale;
        dbg.m_mirrorMapNearPlane = assets.mirrorMapNearPlane;
        dbg.m_mirrorMapFarPlane = assets.mirrorMapFarPlane;
        dbg.m_mirrorMapRenderMirror = assets.mirrorMapRenderMirror;
        dbg.m_mirrorMapRenderWater = assets.mirrorMapRenderWater;

        dbg.m_waterMapEnabled = assets.waterMapEnabled;
        dbg.m_waterMapReflectionBufferScale = assets.waterMapReflectionBufferScale;
        dbg.m_waterMapRefractionBufferScale = assets.waterMapRefractionBufferScale;
        dbg.m_waterMapNearPlane = assets.waterMapNearPlane;
        dbg.m_waterMapFarPlane = assets.waterMapFarPlane;

        dbg.m_cubeMapEnabled = assets.cubeMapEnabled;
        dbg.m_cubeMapBufferScale = assets.cubeMapBufferScale;
        dbg.m_cubeMapNearPlane = assets.cubeMapNearPlane;
        dbg.m_cubeMapFarPlane = assets.cubeMapFarPlane;
        dbg.m_cubeMapRenderMirror = assets.cubeMapRenderMirror;
        dbg.m_cubeMapRenderWater = assets.cubeMapRenderWater;

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

        dbg.m_normalMapEnabled = assets.normalMapEnabled;

        dbg.m_parallaxEnabled = assets.parallaxEnabled;
        dbg.m_parallaxMethod = assets.parallaxMethod;
        dbg.m_parallaxDebugEnabled = assets.parallaxDebugEnabled;
        dbg.m_parallaxDebugDepth = assets.parallaxDebugDepth;

        dbg.m_decalId = SID("graffiti_tag_1");

        dbg.m_drawDebug = assets.drawDebug;

        dbg.m_prepassDepthEnabled = assets.prepassDepthEnabled;

        dbg.m_effectOitEnabled = assets.effectOitEnabled;
        dbg.m_effectOitMinBlendThreshold = assets.effectOitMinBlendThreshold;
        dbg.m_effectOitMaxBlendThreshold = assets.effectOitMaxBlendThreshold;

        dbg.m_effectSsaoEnabled = assets.effectSsaoEnabled;
        dbg.m_effectSsaoBaseColorEnabled = assets.effectSsaoBaseColorEnabled;
        dbg.m_effectSsaoBaseColor = assets.effectSsaoBaseColor;

        dbg.m_effectEmissionEnabled = assets.effectEmissionEnabled;
        dbg.m_effectFogEnabled = assets.effectFogEnabled;

        dbg.m_fogColor = assets.fogColor;
        dbg.m_fogStart = assets.fogStart;
        dbg.m_fogEnd = assets.fogEnd;
        dbg.m_fogDensity = assets.fogDensity;

        dbg.m_gammaCorrectEnabled = assets.gammaCorrectEnabled;
        dbg.m_hardwareCorrectGammaEnabled = assets.hardwareCorrectGammaEnabled;
        dbg.m_gammaCorrect = assets.gammaCorrect;

        dbg.m_hdrToneMappingEnabled = assets.hdrToneMappingEnabled;
        dbg.m_hdrExposure = assets.hdrExposure;

        dbg.m_effectBloomEnabled = assets.effectBloomEnabled;
        dbg.m_effectBloomThreshold = assets.effectBloomThreshold;

        dbg.m_particleEnabled = assets.particleEnabled;
        dbg.m_particleMaxCount = assets.particleMaxCount;

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

    if (assets.useEditor) {
        m_editorFrameInit = std::make_shared<FrameInit>(m_window);
        m_editorFrame = std::make_unique<editor::EditorFrame>(m_window);

        PrepareContext ctx{ m_registry.get()};

        m_editorFrameInit->prepare(ctx);
        m_editorFrame->prepare(ctx);
    }

    return 0;
}

int SampleApp::onUpdate(const ki::RenderClock& clock) {
    auto* scene = m_currentScene.get();
    if (!scene) return 0;

    {
        UpdateContext ctx(
            clock,
            scene->m_registry.get());

        scene->updateRT(ctx);
    }

    {
        auto& dbg = debug::DebugContext::modify();

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
            scene->m_registry.get());

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
            scene->m_registry.get(),
            size.x,
            size.y,
            m_dbg);

        scene->updateViewRT(ctx);
    }

    RenderContext ctx(
        "TOP",
        nullptr,
        clock,
        scene->m_registry.get(),
        scene->getCollection(),
        scene->m_renderData.get(),
        //scene->m_nodeDraw.get(),
        scene->m_batch.get(),
        &cameraNode->m_camera->getCamera(),
        assets.nearPlane,
        assets.farPlane,
        size.x,
        size.y,
        m_dbg);
    {
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

        if (assets.useEditor) {
            m_editorFrame->bind(ctx);
            state.invalidateAll();
        }

        scene->bind(ctx);
        scene->render(ctx);
        scene->unbind(ctx);
    }

    if (assets.useEditor) {
        m_editorFrame->draw(ctx, scene, m_dbg);
    }

    {
        const auto& input = *window->m_input;
        InputState inputState{
            input.isModifierDown(Modifier::CONTROL),
            input.isModifierDown(Modifier::SHIFT),
            input.isModifierDown(Modifier::ALT),
            glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT) != 0,
            glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) != 0,
        };

        if (inputState.mouseRight == GLFW_PRESS &&
            input.allowMouse())
        {
            if (inputState.ctrl)
            {
                shoot(ctx, scene, input, inputState, m_lastInputState);
            }
        }

        if (assets.useEditor) {
            m_editorFrame->processInputs(ctx, scene, input, inputState, m_lastInputState);
        }

        m_lastInputState = inputState;
    }

    if (assets.useEditor) {
        m_editorFrame->render(ctx);
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
                util::sleep(100);
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
            util::sleep(100);
        }
        KI_INFO_OUT("APP: WT stopped!");
    }

    if (m_particleUpdater) {
        KI_INFO_OUT("APP: stopping PS...");

        // NOTE KI wait for worker threads to shutdown
        while (m_particleUpdater->isRunning()) {
            util::sleep(100);
        }
        KI_INFO_OUT("APP: PS stopped!");
    }

    if (m_animationUpdater) {
        KI_INFO_OUT("APP: stopping AS...");

        // NOTE KI wait for worker threads to shutdown
        while (m_animationUpdater->isRunning()) {
            util::sleep(100);
        }
        KI_INFO_OUT("APP: AS stopped!");
    }

    if (m_currentScene) {
        m_currentScene->destroy();
    }

    Engine::onDestroy();

    KI_INFO_OUT("APP: stopped all!");
}

void SampleApp::raycastPlayer(
    const RenderContext& ctx,
    Scene* scene,
    const Input& input,
    const InputState& inputState,
    const InputState& lastInputState)
{
    auto* player = scene->getActiveNode();
    if (!player) return;

    {
        const auto* snapshot = player->getSnapshotRT();
        if (!snapshot) return;

        const auto& hit = physics::PhysicsSystem::get().rayCastClosest(
            snapshot->getWorldPosition(),
            snapshot->getViewFront(),
            100.f,
            physics::mask(physics::Category::npc),
            player->toHandle());

        if (hit.isHit) {
            auto* node = hit.handle.toNode();

            KI_INFO_OUT(fmt::format(
                "PLAYER_HIT: node={}, pos={}, normal={}, depth={}",
                node ? node->getName() : "N/A",
                hit.pos,
                hit.normal,
                hit.depth));
        }
    }

    {
        glm::vec2 screenPos{ input.mouseX, input.mouseY };

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

        const auto& hit = physics::PhysicsSystem::get().rayCastClosest(
            startPos,
            dir,
            400.f,
            physics::mask(physics::Category::npc, physics::Category::prop),
            //physics::mask(physics::Category::all),
            player->toHandle());
    }
}

void SampleApp::shoot(
    const RenderContext& ctx,
    Scene* scene,
    const Input& input,
    const InputState& inputState,
    const InputState& lastInputState)
{
    auto* player = scene->getActiveNode();
    if (!player) return;

    {
        const auto& dbg = debug::DebugContext::get();

        glm::vec2 screenPos{ input.mouseX, input.mouseY };

        const auto startPos = ctx.unproject(screenPos, .01f);
        const auto endPos = ctx.unproject(screenPos, .8f);
        const auto dir = glm::normalize(endPos - startPos);

        //const auto& hits = physics::PhysicsSystem::get().rayCast(
        //    startPos,
        //    dir,
        //    400.f,
        //    physics::mask(physics::Category::ray_player_fire),
        //    physics::mask(physics::Category::npc, physics::Category::prop),
        //    //physics::mask(physics::Category::all),
        //    player->toHandle(),
        //    true);

        auto callback = [this](int cid, const physics::RayHit& hits) {
            shootCallback(hits);
        };

        auto& commandEngine = script::CommandEngine::get();
        commandEngine.addCommand(
            0,
            script::RayCast{
                player->toHandle(),
                dir,
                400.f,
                physics::mask(physics::Category::npc, physics::Category::prop, physics::Category::terrain),
                false,
                callback
            });


        g_hitElapsed += ctx.m_clock.elapsedSecs;
    }
}

void SampleApp::shootCallback(
    const physics::RayHit& hit
)
{
    auto* player = m_currentScene->getActiveNode();
    if (!player) return;

    {
        const auto& dbg = debug::DebugContext::get();

        if (hit.isHit && g_hitElapsed >= HIT_RATE) {
            g_hitElapsed -= HIT_RATE;

            {
                auto* node = hit.handle.toNode();
                KI_INFO_OUT(fmt::format(
                    "SCREEN_HIT: node={}, pos={}, normal={}, depth={}",
                    node ? node->getName() : "N/A",
                    hit.pos,
                    hit.normal,
                    hit.depth));

                auto sid = dbg.m_decalId;
                auto df = decal::DecalRegistry::get().getDecal(sid);
                KI_INFO_OUT(fmt::format("DECAL: name={}, valid={}", sid.str(), df ? true : false));

                auto decal = df.createForHit(hit.handle, hit.pos, glm::normalize(hit.normal));

                decal::DecalSystem::get().addDecal(decal);

                KI_INFO_OUT(fmt::format(
                    "DECAL: node={}, pos={}, normal={}",
                    node ? node->getName() : "N/A",
                    decal.m_position,
                    decal.m_normal));
            }
        }
    }
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
            scene->m_name = assets.sceneFile;

            auto ctx = std::make_shared<loader::Context>(
                m_alive,
                m_asyncLoader,
                assets.sceneDir,
                assets.sceneFile
            );
            std::unique_ptr<loader::SceneLoader> loader = std::make_unique<loader::SceneLoader>(ctx);
            m_loaders.push_back(std::move(loader));
        }
    }

    for (auto& loader : m_loaders) {
        KI_INFO_OUT(fmt::format("LOAD_SCENE: {}", loader->m_ctx->str()));
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
