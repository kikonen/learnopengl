	#include "Test6.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Mesh.h"

#include "KIGL.h"

Test6::Test6() {
	title = "Test 6";
	assets.shadersDir = "shader/test6";
	//throttleFps = 0;
	//glfwWindowHint(GLFW_SAMPLES, 4);
}

int Test6::onSetup() {
	SceneSetup1* sceneSetup = setupScene1();
	currentScene = sceneSetup;

	camera.setPos(glm::vec3(-8, 5, 10.f) + assets.groundOffset);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glDisable(GL_MULTISAMPLE);

	KIGL::startError();
	KIGL::startDebug();

	return 0;
}

int Test6::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	Scene* scene = currentScene->scene;

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// https://cmichel.io/understanding-front-faces-winding-order-and-normals
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	const glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)width / (float)height, 0.1f, 1000.0f);

	RenderContext ctx(
		*this, dt, 
		camera.getView(), 
		projection,
		scene->dirLight, scene->pointLights, scene->spotLights);
	//ctx.useWireframe = true;
	//ctx.useLight = false;

	currentScene->process(ctx);

	currentScene->update(ctx);
	currentScene->bind(ctx);
	currentScene->draw(ctx);

	return 0;
}

void Test6::processInput(float dt) {
	Engine::processInput(dt);
}

SceneSetup1* Test6::setupScene1()
{
	SceneSetup1* sceneSetup = new SceneSetup1(assets, ubo);
	sceneSetup->setup();

	//sceneSetup->scene->showNormals = true;
	sceneSetup->scene->prepare();

	return sceneSetup;
}

