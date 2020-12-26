#include "Test4.h"

Test4::Test4() {
	title = "Test 4";
	//   throttleFps = FPS_30;
}

int Test4::onSetup() {
	mesh = new ModelMesh("texture_cube");
	if (mesh->load()) {
		return -1;
	}

	return 0;
}

int Test4::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// tri 1
//	mesh->bind(dt);
//	mesh->shader->use();
//	mesh->draw(dt);
	glBindVertexArray(0);

	return 0;
}

