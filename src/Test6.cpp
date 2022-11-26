#include "Test6.h"

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "editor/EditorFrame.h"

#include "TestSceneSetup.h"

#include "AssetsFile.h"
#include "SceneFile.h"

#include "controller/VolumeController.h"


Test6::Test6()
{
}

int Test6::onInit()
{
    m_title = "Test 6";
    //glfwWindowHint(GLFW_SAMPLES, 4);

    m_assets = loadAssets();
    return 0;
}

int Test6::onSetup() {
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

int Test6::onRender(const ki::RenderClock& clock) {
    auto* scene = m_currentScene.get();
    Window* window = m_window.get();

    if (!scene) return 0;

    scene->attachNodes();

    Camera* camera = scene->getCamera();
    if (!camera) return 0;

    int w = window->m_width;
    int h = window->m_height;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    RenderContext ctx("TOP", nullptr,
        m_assets, clock, m_state, scene, *camera, m_backend.get(),
        m_assets.nearPlane, m_assets.farPlane, w, h);
    //ctx.useWireframe = true;
    //ctx.useLight = false;

    //// https://cmichel.io/understanding-front-faces-winding-order-and-normals
    //ctx.state.enable(GL_CULL_FACE);
    //ctx.state.cullFace(GL_BACK);
    //ctx.state.frontFace(GL_CCW);

    //ctx.state.enable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);

    // https://paroj.github.io/gltut/apas04.html
    //glEnable(GL_RASTERIZER_DISCARD);

    ctx.state.enable(GL_PROGRAM_POINT_SIZE);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

    if (m_assets.useIMGUI) {
        m_frame->bind(ctx);
    }

    scene->processEvents(ctx);
    scene->update(ctx);
    scene->bind(ctx);
    scene->draw(ctx);
    scene->unbind(ctx);

    bool isCtrl = window->m_input->isModifierDown(Modifier::CONTROL);
    bool isShift = window->m_input->isModifierDown(Modifier::SHIFT);
    int state = glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT);

    if ((isCtrl && state == GLFW_PRESS) && (!m_assets.useIMGUI || !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))) {
        selectNode(ctx, scene, isShift, isCtrl);
    }

    if (m_assets.useIMGUI) {
        //ImGui::ShowDemoWindow();

        m_frame->draw(ctx);
        m_frame->render(ctx);
    }

    m_drawCount += ctx.m_drawCount;
    m_skipCount += ctx.m_skipCount;

    if (m_assets.frustumEnabled && m_assets.frustumDebug) {
        m_frustumElapsedSecs += clock.elapsedSecs;
        if (m_frustumElapsedSecs >= 10) {
            m_frustumElapsedSecs -= 10;
            auto ratio = (float)m_skipCount / (float)m_drawCount;
            auto frameDraw = (float)m_drawCount / (float)clock.frameCount;
            auto frameSkip = (float)m_skipCount / (float)clock.frameCount;
            KI_INFO_SB(fmt::format("{} : total-frames: {}, total-draw: {}, total-skip: {}, ratio: {}", ctx.m_name, clock.frameCount, m_drawCount, m_skipCount, ratio));
            KI_INFO_SB(fmt::format("{} : frame-draw: {}, frame-skip: {}", ctx.m_name, frameDraw, frameSkip));
        }
    }

    return 0;
}

void Test6::onDestroy()
{
}

void Test6::selectNode(
    const RenderContext& ctx,
    Scene* scene,
    bool isShift,
    bool isCtrl)
{
    auto& registry = scene->m_registry;
    int objectID = scene->getObjectID(ctx, m_window->m_input->mouseX, m_window->m_input->mouseY);

    auto* volumeNode = registry.getNode(ctx.assets.volumeUUID);
    auto* node = registry.getNode(objectID);

    if (false && node && volumeNode && node->m_selected) {
        node->m_selected = false;

        volumeNode->setPosition({0, 0, 0});
        volumeNode->setScale(1.f);

        return;
    }

    if (!volumeNode || volumeNode->m_objectID != objectID) {
        registry.selectNodeByObjectId(objectID, isShift);

        if (volumeNode && node) {
            auto controller = dynamic_cast<VolumeController*>(volumeNode->m_controller.get());
            controller->setTarget(node->m_objectID);
        }

        KI_INFO_SB("selected: " << objectID);
    }
}

Assets Test6::loadAssets()
{
    AssetsFile file{ "scene/assets.yml" };
    return file.load();
}

std::shared_ptr<Scene> Test6::loadScene()
{
    auto scene = std::make_shared<Scene>(m_assets);

    m_asyncLoader->m_scene = scene;

    m_file = std::make_unique<SceneFile>(m_asyncLoader.get(), m_assets, "scene/scene_full.yml");
    //m_file = std::make_unique<SceneFile>(m_asyncLoader.get(), m_assets, "scene/scene_player.yml");
    //m_file = std::make_unique<SceneFile>(m_asyncLoader.get(), m_assets, "scene/scene_origo.yml");
    //m_file = std::make_unique<SceneFile>(m_asyncLoader.get(), m_assets, "scene/scene_water.yml");
    m_file->load(scene);

    m_testSetup = std::make_unique<TestSceneSetup>(m_asyncLoader.get(), m_assets);
    m_testSetup->setup(scene);

    scene->prepare(m_shaders);

    return scene;
}

void save() {
}
