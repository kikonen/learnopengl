#include "Test6.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"
#include "EditorFrame.h"

#include "SceneLoaderTest.h"

Test6::Test6() {
	title = "Test 6";
	assets.shadersDir = "shader/test6";
	//throttleFps = 0;
	//throttleFps = FPS_60;
	//glfwWindowHint(GLFW_SAMPLES, 4);
}

int Test6::onSetup() {
	SceneLoader* loader = loadScene();
	currentScene = loader->scene;

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glDisable(GL_MULTISAMPLE);

	ki::GL::startError();
	ki::GL::startDebug();

	frameInit = new FrameInit(*window);
	frame = new EditorFrame(*window);

	return 0;
}

int Test6::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// https://cmichel.io/understanding-front-faces-winding-order-and-normals
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	RenderContext ctx(*this, dt, currentScene, currentScene->getCamera(), window->width, window->height);
	//ctx.useWireframe = true;
	//ctx.useLight = false;

	frame->bind(ctx);

	currentScene->processEvents(ctx);
	currentScene->update(ctx);
	currentScene->bind(ctx);
	currentScene->draw(ctx);

	frame->draw(ctx);
	frame->render(ctx);

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

	//sceneSetup->scene->showNormals = true;
	//loader->load();
	loader->scene->prepare();

	return loader;
}

void save() {

}
