#include "Test5.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

Test5::Test5() {
	title = "Test 5";
	//throttleFps = FPS_30;
}

int Test5::onSetup() {
	mesh = new ModelMesh(*this, "texture_cube", "test5");
	if (mesh->load()) {
		return -1;
	}

	mesh->prepare();

	return 0;
}

int Test5::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	int w = 0;
	int h = 0;
	glfwGetWindowSize(window, &w, &h);

	glm::mat4 view = camera.updateCamera(dt);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);

	const glm::mat4 vpMat = projection * view;

//	mesh->setPos(glm::vec3(0, 0, -10.0f));

	// tri 1
	if (false) {
		const float radius = 4.0f;
		float posX = sin(accumulatedTime / 0.9f) * radius;
		float posY = sin(accumulatedTime * 1.1f) * radius / 3.0f;
		float posZ = cos(accumulatedTime) * radius / 2.0f;

		mesh->setPos(glm::vec3(posX, posY, posZ));
	}
	if (false) {
		const float radius = 2.0f;
		float rotX = accumulatedTime * radius;
		float rotY = accumulatedTime * radius * 1.1f;
		float rotZ = accumulatedTime * radius * 1.2f;

		mesh->setRotation(glm::vec3(rotX, rotY, rotZ));
	}

	if (false) {
		const float radius = 2.0f;
		float scale = sin(accumulatedTime / 4.0f) * radius;

		mesh->setScale(scale);
	}

	mesh->bind(dt, vpMat);
	mesh->draw(dt, vpMat);
	glBindVertexArray(0);

	return 0;
}

