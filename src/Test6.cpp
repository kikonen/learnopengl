#include "Test6.h"

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "editor/EditorFrame.h"

#include "TestSceneSetup.h"

#include "AssetsFile.h"
#include "SceneFile.h"


Test6::Test6()
{
}

int Test6::onInit()
{
    title = "Test 6";
    //glfwWindowHint(GLFW_SAMPLES, 4);

    assets = loadAssets();
    return 0;
}

int Test6::onSetup() {
    currentScene = loadScene();

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (assets.glfwSwapInterval >= 0) {
        glfwSwapInterval(assets.glfwSwapInterval);
    }

    //state.disable(GL_MULTISAMPLE);

    if (assets.useIMGUI) {
        frameInit = std::make_unique<FrameInit>(*window);
        frame = std::make_unique<EditorFrame>(*window);
    }

    return 0;
}

int Test6::onRender(const RenderClock& clock) {
    auto* scene = currentScene.get();
    Window* window = this->window.get();

    if (!scene) return 0;

    scene->attachNodes();

    Camera* camera = scene->getCamera();
    if (!camera) return 0;

    int w = window->width;
    int h = window->height;
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    RenderContext ctx("TOP", nullptr,
        assets, clock, state, scene, *camera, assets.nearPlane, assets.farPlane, w, h);
    //ctx.useWireframe = true;
    //ctx.useLight = false;

    //// https://cmichel.io/understanding-front-faces-winding-order-and-normals
    //ctx.state.enable(GL_CULL_FACE);
    //ctx.state.cullFace(GL_BACK);
    //ctx.state.frontFace(GL_CCW);

    //ctx.state.enable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);

    if (assets.useIMGUI) {
        frame->bind(ctx);
    }

    scene->processEvents(ctx);
    scene->update(ctx);
    scene->bind(ctx);
    scene->draw(ctx);
    scene->unbind(ctx);

    bool isCtrl = window->input->isModifierDown(Modifier::CONTROL);
    bool isShift = window->input->isModifierDown(Modifier::SHIFT);
    int state = glfwGetMouseButton(window->glfwWindow, GLFW_MOUSE_BUTTON_LEFT);

    if ((isCtrl && state == GLFW_PRESS) && (!assets.useIMGUI || !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))) {
        int objectID = scene->getObjectID(ctx, window->input->mouseX, window->input->mouseY);

        scene->registry.selectNodeByObjectId(objectID, isShift);

        KI_INFO_SB("selected: " << objectID);
    }

    if (assets.useIMGUI) {
        //ImGui::ShowDemoWindow();

        frame->draw(ctx);
        frame->render(ctx);
    }

    drawCount += ctx.drawCount;
    skipCount += ctx.skipCount;

    if (assets.frustumEnabled && assets.frustumDebug) {
        frustumElapsedSecs += clock.elapsedSecs;
        if (frustumElapsedSecs >= 10) {
            frustumElapsedSecs -= 10;
            auto ratio = (float)skipCount / (float)drawCount;
            auto frameDraw = (float)drawCount / (float)clock.frameCount;
            auto frameSkip = (float)skipCount / (float)clock.frameCount;
            KI_INFO_SB(fmt::format("{} : total-frames: {}, total-draw: {}, total-skip: {}, ratio: {}", ctx.name, clock.frameCount, drawCount, skipCount, ratio));
            KI_INFO_SB(fmt::format("{} : frame-draw: {}, frame-skip: {}", ctx.name, frameDraw, frameSkip));
        }
    }

    return 0;
}

void Test6::onDestroy()
{
}

Assets Test6::loadAssets()
{
    AssetsFile file{ "scene/assets.yml" };
    return file.load();
}

std::shared_ptr<Scene> Test6::loadScene()
{
    auto scene = std::make_shared<Scene>(assets);

    asyncLoader->scene = scene;

    file = std::make_unique<SceneFile>(asyncLoader.get(), assets, "scene/scene_full.yml");
    file->load(scene);

    testSetup = std::make_unique<TestSceneSetup>(asyncLoader.get(), assets);
    testSetup->setup(scene);

    //loader.scene->showNormals = true;
    //loader.scene->showMirrorView = true;
    //loader.load();
    scene->prepare(shaders);

    return scene;
}

void save() {
}
