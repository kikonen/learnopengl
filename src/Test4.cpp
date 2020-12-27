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
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	// tri 1
	mesh->bind(camera, dt);
	mesh->draw(camera, dt);
	glBindVertexArray(0);

	return 0;
}

