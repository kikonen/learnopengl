#include "Test4.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

Test4::Test4() {
	title = "Test 4";
	//throttleFps = 0;
}

int Test4::onSetup() {
	mesh = new ModelMesh("cow");
	if (mesh->load()) {
		return -1;
	}

	mesh->prepare();

	return 0;
}

int Test4::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glm::mat4 view = camera.updateCamera(dt);

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	std::string projectionName = { "projection" };
	mesh->shader->setMat4(projectionName, projection);

	std::string viewName = { "view" };
	mesh->shader->setMat4(viewName, view);

	// tri 1
	mesh->bind(camera, dt);
	mesh->draw(camera, dt);
	glBindVertexArray(0);

	return 0;
}

