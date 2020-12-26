#include "Test4.h"

#include <glm/glm.hpp>

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
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
	
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

/*	glm::mat4 view;
	view = glm::lokAt(glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
		*/

	// tri 1
	mesh->bind(dt);
	mesh->shader->use();
	mesh->draw(dt);
	glBindVertexArray(0);

	return 0;
}

