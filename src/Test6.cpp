#include "Test6.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "editor/EditorFrame.h"

#include "SceneLoaderTest.h"

Test6::Test6() {
	title = "Test 6";
	assets.shadersDir = "shader/test6";
	throttleFps = 0;
	//throttleFps = FPS_30;
	//glfwWindowHint(GLFW_SAMPLES, 4);

	useIMGUI = false;
}

int Test6::onSetup() {
	SceneLoader* loader = loadScene();
	currentScene = loader->scene;

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSwapInterval(3);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//state.disable(GL_MULTISAMPLE);

	if (useIMGUI) {
		frameInit = new FrameInit(*window);
		frame = new EditorFrame(*window);
	}

	return 0;
}

int Test6::onRender(const RenderClock& clock) {
	RenderContext ctx(assets, clock, state, currentScene, currentScene->getCamera(), window->width, window->height);
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

	//ImGui::ShowDemoWindow();

	if (useIMGUI) {
		frame->draw(ctx);
		frame->render(ctx);
	}

	return 0;
}

void Test6::onDestroy()
{
}

SceneLoader* Test6::loadScene()
{
	assets.batchSize = 1000;

	loader = new SceneLoaderTest(assets);
	loader->setup();

	//loader->scene->showNormals = true;
	//loader->scene->showMirrorView = true;
	//loader->load();
	loader->scene->prepare();

	return loader;
}

void save() {

}
