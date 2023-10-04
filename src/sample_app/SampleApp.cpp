#include "SampleApp.h"

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "kigl/kigl.h"

#include "editor/EditorFrame.h"

#include "asset/DynamicCubeMap.h"

#include "backend/gl/PerformanceCounters.h"

#include "controller/VolumeController.h"

#include "registry/MeshType.h"
#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"

#include "engine/AssetsFile.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "scene/Scene.h"
#include "scene/SceneFile.h"

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

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (m_assets.glfwSwapInterval >= 0) {
        glfwSwapInterval(m_assets.glfwSwapInterval);
    }

    //state.setEnabled(GL_MULTISAMPLE, false);

    if (m_assets.useIMGUI) {
        m_frameInit = std::make_unique<FrameInit>(*m_window);
        m_frame = std::make_unique<EditorFrame>(*m_window);
    }

    return 0;
}

int SampleApp::onUpdate(const ki::RenderClock& clock) {
    auto* scene = m_currentScene.get();
    if (!scene) return 0;

    {
        UpdateContext ctx(
            clock,
            m_assets,
            m_currentScene->m_commandEngine.get(),
            m_currentScene->m_scriptEngine.get(),
            m_currentScene->m_registry.get());

        scene->processEvents(ctx);

        scene->update(ctx);
    }

    return 0;
}

int SampleApp::onRender(const ki::RenderClock& clock) {
    auto* scene = m_currentScene.get();
    Window* window = m_window.get();

    if (!scene) return 0;

    Node* cameraNode = scene->getActiveCamera();
    if (!cameraNode) return 0;


    glm::vec2 size = window->getSize();

    RenderContext ctx(
        "TOP",
        nullptr,
        clock,
        m_assets,
        m_currentScene->m_commandEngine.get(),
        m_currentScene->m_scriptEngine.get(),
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
        }
        m_state.clear();

        scene->updateView(ctx);
        scene->bind(ctx);
        scene->draw(ctx);
        scene->unbind(ctx);
    }

    {
        InputState state{
            window->m_input->isModifierDown(Modifier::CONTROL),
            window->m_input->isModifierDown(Modifier::SHIFT),
            glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT),
            glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_RIGHT),
        };

        if (state.mouseLeft != m_lastInputState.mouseLeft &&
            state.mouseLeft == GLFW_PRESS &&
            state.ctrl)
        {
            if (state.ctrl && (!m_assets.useIMGUI || !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))) {
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

            KI_INFO(fmt::format(
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
}

void SampleApp::selectNode(
    const RenderContext& ctx,
    Scene* scene,
    const InputState& inputState,
    const InputState& lastInputState)
{
    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;
    int objectID = scene->getObjectID(ctx, m_window->m_input->mouseX, m_window->m_input->mouseY);

    auto* volumeNode = nodeRegistry.getNode(ctx.m_assets.volumeUUID);
    auto* node = nodeRegistry.getNode(objectID);

    // deselect
    if (node && node->isSelected()) {
        nodeRegistry.selectNodeByObjectId(-1, false);

        if (volumeNode) {
            auto* controller = ctx.m_registry->m_controllerRegistry->get<VolumeController>(volumeNode);
            if (controller->getTarget() == node->m_objectID) {
                controller->setTarget(-1);
            }
        }

        return;
    }

    // select
    nodeRegistry.selectNodeByObjectId(objectID, inputState.shift);

    if (volumeNode) {
        auto* controller = ctx.m_registry->m_controllerRegistry->get<VolumeController>(volumeNode);
        controller->setTarget(node ? node->m_objectID : -1);
    }

    KI_INFO(fmt::format("selected: {}", objectID));

    if (node) {
        event::Event evt { event::Type::animate_rotate };
        evt.body.animate = {
            .target = node->m_objectID,
            .duration = 5,
            .data = { 0, 360.f, 0 }
        };
        ctx.m_registry->m_dispatcher->send(evt);
    }

    if (node) {
        nodeRegistry.setActiveCamera(node);
    }
    else {
        nodeRegistry.setActiveCamera(nodeRegistry.findDefaultCamera());
    }
}

Assets SampleApp::loadAssets()
{
    AssetsFile file{ "scene/assets.yml" };
    return file.load();
}

std::shared_ptr<Scene> SampleApp::loadScene()
{
    auto scene = std::make_shared<Scene>(m_assets, m_registry);

    auto& alive = scene->m_alive;

    {
        if (!m_assets.sceneFile.empty()) {
            std::unique_ptr<SceneFile> file = std::make_unique<SceneFile>(m_assets, alive, m_asyncLoader, m_assets.sceneFile);
            m_files.push_back(std::move(file));
        }
    }

    for (auto& file : m_files) {
        KI_INFO_OUT(fmt::format("LOAD_SCENE: {}", file->m_filename));
        file->load(m_registry);
    }

    m_testSetup = std::make_unique<TestSceneSetup>(
        m_assets,
        alive,
        m_asyncLoader);

    m_testSetup->setup(
        scene->m_registry
    );

    scene->prepare();

    return scene;
}

void save() {
}
