#include "SampleApp.h"

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "editor/EditorFrame.h"

#include "backend/gl/PerformanceCounters.h"

#include "controller/VolumeController.h"

#include "registry/MeshType.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/MaterialRegistry.h"

#include "TestSceneSetup.h"

#include "AssetsFile.h"
#include "SceneFile.h"

#include "scene/Scene.h"


SampleApp::SampleApp()
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

    //state.disable(GL_MULTISAMPLE);

    if (m_assets.useIMGUI) {
        m_frameInit = std::make_unique<FrameInit>(*m_window);
        m_frame = std::make_unique<EditorFrame>(*m_window);
    }

    return 0;
}

int SampleApp::onRender(const ki::RenderClock& clock) {
    auto* scene = m_currentScene.get();
    Window* window = m_window.get();

    if (!scene) return 0;

    scene->attachNodes();

    Node* cameraNode = scene->getActiveCamera();
    if (!cameraNode) return 0;

    int w = window->m_width;
    int h = window->m_height;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    RenderContext ctx("TOP", nullptr,
        m_assets, clock, m_state, scene, cameraNode->m_camera.get(), m_backend.get(),
        m_assets.nearPlane, m_assets.farPlane, w, h);

    ctx.m_forceWireframe = m_assets.forceWireframe;
    ctx.m_useLight = m_assets.useLight;

    // https://paroj.github.io/gltut/apas04.html
    if (m_assets.rasterizerDiscard) {
        //glEnable(GL_RASTERIZER_DISCARD);
        ctx.state.enable(GL_RASTERIZER_DISCARD);
    }

    ctx.state.useProgram(0);
    ctx.state.bindVAO(0);

    ctx.state.enable(GL_PROGRAM_POINT_SIZE);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

    if (m_assets.useIMGUI) {
        m_frame->bind(ctx);
    }

    //std::cout << "*****" << "\n";
    scene->processEvents(ctx);
    scene->update(ctx);
    scene->bind(ctx);
    scene->draw(ctx);
    scene->unbind(ctx);
    //std::cout << "\n*****" << "\n";

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
        //ImGui::ShowDemoWindow();

        m_frame->draw(ctx);
        m_frame->render(ctx);
    }

    if (m_assets.frustumDebug) {
        m_frustumElapsedSecs += clock.elapsedSecs;
        if (m_frustumElapsedSecs >= 10) {
            m_frustumElapsedSecs -= 10;

            auto counters = scene->getCounters(true);
            m_drawCount += counters.u_drawCount;
            m_skipCount += counters.u_skipCount;

            auto countersLocal = scene->getCountersLocal(true);

            KI_INFO_OUT(fmt::format("BATCH : batch-draw: {}, batch-skip: {}", countersLocal.u_drawCount, countersLocal.u_skipCount));

            auto ratio = (float)m_skipCount / (float)m_drawCount;
            auto frameDraw = (float)m_drawCount / (float)clock.frameCount;
            auto frameSkip = (float)m_skipCount / (float)clock.frameCount;

            KI_INFO(fmt::format("{} : total-frames: {}, total-draw: {}, total-skip: {}, ratio: {}", ctx.m_name, clock.frameCount, m_drawCount, m_skipCount, ratio));
            KI_INFO(fmt::format("{} : frame-draw: {}, frame-skip: {}", ctx.m_name, frameDraw, frameSkip));
        }
    }

    return 0;
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

    auto* volumeNode = nodeRegistry.getNode(ctx.assets.volumeUUID);
    auto* node = nodeRegistry.getNode(objectID);

    // deselect
    if (node && node->isSelected()) {
        node->setSelectionMaterialIndex(-1);

        if (volumeNode) {
            auto controller = dynamic_cast<VolumeController*>(volumeNode->m_controller.get());
            if (controller->getTarget() == node->m_objectID) {
                controller->setTarget(-1);
            }
        }

        return;
    }

    // select
    nodeRegistry.selectNodeByObjectId(objectID, inputState.shift);

    if (volumeNode) {
        auto controller = dynamic_cast<VolumeController*>(volumeNode->m_controller.get());
        controller->setTarget(node ? node->m_objectID : -1);
    }

    KI_INFO(fmt::format("selected: {}", objectID));

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