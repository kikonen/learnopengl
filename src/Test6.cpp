#include "Test6.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "editor/EditorFrame.h"

#include "TestSceneSetup.h"
#include "SceneFile.h"


Test6::Test6()
{
    title = "Test 6";
    assets.shadersDir = "shader/test6";
    throttleFps = 0;
    //throttleFps = FPS_30;
    //glfwWindowHint(GLFW_SAMPLES, 4);

    useIMGUI = true;
}

int Test6::onSetup() {
    currentScene = loadScene();

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(3);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //state.disable(GL_MULTISAMPLE);

    if (useIMGUI) {
        frameInit = std::make_unique<FrameInit>(*window);
        frame = std::make_unique<EditorFrame>(*window);
    }

    return 0;
}

int Test6::onRender(const RenderClock& clock) {
    currentScene->attachNodes();

    Camera* camera = currentScene->getCamera();
    if (!camera) {
        return 0;
    }

    RenderContext ctx(assets, clock, state, currentScene, *camera, window->width, window->height);
    //ctx.useWireframe = true;
    //ctx.useLight = false;

    // https://cmichel.io/understanding-front-faces-winding-order-and-normals
    ctx.state.enable(GL_CULL_FACE);
    ctx.state.cullFace(GL_BACK);
    ctx.state.frontFace(GL_CCW);

    ctx.state.enable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    if (useIMGUI) {
        frame->bind(ctx);
    }

    currentScene->processEvents(ctx);
    currentScene->update(ctx);
    currentScene->bind(ctx);
    currentScene->draw(ctx);

    bool isCtrl = window->input->isModifierDown(Modifier::CONTROL);
    bool isShift = window->input->isModifierDown(Modifier::SHIFT);
    int state = glfwGetMouseButton(window->glfwWindow, GLFW_MOUSE_BUTTON_LEFT);

    if ((isCtrl && state == GLFW_PRESS) && (!useIMGUI || !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))) {
        int objectID = currentScene->getObjectID(ctx, window->input->mouseX, window->input->mouseY);

        currentScene->registry.selectNodeById(objectID, isShift);

        KI_INFO_SB("selected: " << objectID);
    }

    if (useIMGUI) {
        //ImGui::ShowDemoWindow();

        frame->draw(ctx);
        frame->render(ctx);
    }

    return 0;
}

void Test6::onDestroy()
{
}

std::shared_ptr<Scene> Test6::loadScene()
{
    assets.batchSize = 1000;

    auto scene = std::make_shared<Scene>(assets);

    asyncLoader->scene = scene;

    file = std::make_shared<SceneFile>(asyncLoader, assets, "scene/scene_full.yml");
    file->load(scene);

    testSetup = std::make_shared<TestSceneSetup>(asyncLoader, assets);
    testSetup->setup(scene);

    //loader.scene->showNormals = true;
    //loader.scene->showMirrorView = true;
    //loader.load();
    scene->prepare(shaders);

    return scene;
}

void save() {
}
