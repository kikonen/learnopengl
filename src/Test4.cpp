#include "Test4.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

Test4::Test4() {
	title = "Test 4";
	throttleFps = FPS_30;
}

int Test4::onSetup() {
	mesh = new ModelMesh("texture_ball");
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

	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
	std::string lightPosName = { "lightPos" };
	mesh->shader->setFloat3(lightPosName, lightPos.x, lightPos.y, lightPos.z);

	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	std::string lightColorName = { "lightColor" };
	mesh->shader->setFloat3(lightColorName, 0.8f, 0.8f, 0.1f);

	glm::mat4 view = camera.updateCamera(dt);

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	std::string projectionName = { "projection" };
	mesh->shader->setMat4(projectionName, projection);

	std::string viewName = { "view" };
	mesh->shader->setMat4(viewName, view);

	// tri 1
	if (true) {
		const float radius = 4.0f;
		float posX = sin(accumulatedTime / 0.9) * radius;
		float posY = sin(accumulatedTime * 1.1) * radius / 3;
		float posZ = cos(accumulatedTime) * radius / 2;

		mesh->setPos(glm::vec3(posX, posY, posZ));
	}
	if (true) {
		const float radius = 2.0f;
		float rotX = accumulatedTime* radius;
		float rotY = accumulatedTime* radius * 1.1;
		float rotZ = accumulatedTime* radius * 1.2;

		mesh->setRotation(glm::vec3(rotX, rotY, rotZ));
	}

	if (true) {
		const float radius = 2.0f;
		float scale = sin(accumulatedTime / 4) * radius;

		mesh->setScale(scale);
	}

	mesh->bind(camera, dt);
	mesh->draw(camera, dt);
	glBindVertexArray(0);

	return 0;
}

