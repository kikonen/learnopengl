#include "Test4.h"

#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ModelMesh.h"

Test4::Test4() {
	title = "Test 4";
	//throttleFps = FPS_30;
}

int Test4::onSetup() {
	ModelMesh* mesh = new ModelMesh(*this, "texture_cube", "test4");
	if (mesh->load()) {
		return -1;
	}

	node = new Node(mesh, glm::vec3(0));

	return 0;
}

int Test4::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	Shader* shader = nullptr; // ->mesh->shader;

	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
	std::string lightPosName = { "lightPos" };
	shader->setFloat3(lightPosName, lightPos.x, lightPos.y, lightPos.z);

	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	std::string lightColorName = { "lightColor" };
	shader->setFloat3(lightColorName, 0.8f, 0.8f, 0.1f);

	glm::mat4 view = camera.getView();

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	std::vector<Light*> pointLights;
	std::vector<Light*> spotLights;
	RenderContext ctx(*this, dt, view, projection, 0, nullptr, pointLights, spotLights);

	std::string projectionName = { "projection" };
	shader->setMat4(projectionName, projection);

	std::string viewName = { "view" };
	shader->setMat4(viewName, view);

	// tri 1
	if (false) {
		const float radius = 4.0f;
		float posX = sin(accumulatedTime / 0.9f) * radius;
		float posY = sin(accumulatedTime * 1.1f) * radius / 3;
		float posZ = cos(accumulatedTime) * radius / 2.0f;

		node->setPos(glm::vec3(posX, posY, posZ));
	}
	if (false) {
		const float radius = 2.0f;
		float rotX = accumulatedTime* radius;
		float rotY = accumulatedTime* radius * 1.1f;
		float rotZ = accumulatedTime* radius * 1.2f;

		node->setRotation(glm::vec3(rotX, rotY, rotZ));
	}

	if (false) {
		const float radius = 2.0f;
		float scale = sin(accumulatedTime / 4) * radius;

		node->setScale(scale);
	}

	node->draw(ctx);
	glBindVertexArray(0);

	return 0;
}

