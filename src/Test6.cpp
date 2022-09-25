#include "Test6.h"

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


    RenderContext ctx(assets, clock, state, scene, *camera, window->width, window->height);
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

        scene->registry.selectNodeById(objectID, isShift);

        KI_INFO_SB("selected: " << objectID);
    }

    if (assets.useIMGUI) {
        //ImGui::ShowDemoWindow();

        frame->draw(ctx);
        frame->render(ctx);
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
