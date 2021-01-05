#include "Test6.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ModelMesh.h"

Test6::Test6() {
	title = "Test 5";
	//throttleFps = 0;
}

int Test6::onSetup() {
	// light
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "light", "test6");
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(0, 4, 0));
		//node->setScale(0.5f);
		nodes.push_back(node);

		light = new Light();
		light->pos = node->getPos();
	}

	// mountains
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "mountains", "test6");
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(0, -20, -20));
		//		node->setScale(0.01);
		nodes.push_back(node);
	}

	// active
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "texture_cube", "test6");
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		active = new Node(mesh, glm::vec3(0));
		nodes.push_back(active);

		nodes.push_back(new Node(mesh, glm::vec3(5, 5, 5)));
	}

	// cubes
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "texture_cube_3", "test6");
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		nodes.push_back(new Node(mesh, glm::vec3(-5, 0, -5)));
		nodes.push_back(new Node(mesh, glm::vec3(5, 0, -5)));
		nodes.push_back(new Node(mesh, glm::vec3(-5, 0, 5)));
		nodes.push_back(new Node(mesh, glm::vec3(5, 0, 5)));
	}

	// ball
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "texture_ball", "test6");
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(0, -2, 0));
		node->setScale(2.0f);
		nodes.push_back(node);
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return 0;
}

int Test6::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_DEPTH_TEST);

	int w = 0;
	int h = 0;
	glfwGetWindowSize(window, &w, &h);

	const glm::mat4& view = camera.getView();
	const glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)w / (float)h, 0.1f, 1000.0f);

	RenderContext ctx(*this, dt, view, projection, nullptr);

	//	mesh->setPos(glm::vec3(0, 0, -10.0f));

	if (active) {
		if (true) {
			const float radius = 4.0f;
			float posX = sin(accumulatedTime / 0.9f) * radius;
			float posY = sin(accumulatedTime * 1.1f) * radius / 3.0f;
			float posZ = cos(accumulatedTime) * radius / 2.0f;

			active->setPos(glm::vec3(posX, posY, posZ));
		}

		if (true) {
			const float radius = 2.0f;
			float rotX = accumulatedTime * radius;
			float rotY = accumulatedTime * radius * 1.1f;
			float rotZ = accumulatedTime * radius * 1.2f;

			active->setRotation(glm::vec3(rotX, rotY, rotZ));
		}

		if (true) {
			const float radius = 2.0f;
			float scale = sin(accumulatedTime / 4.0f) * radius;

			active->setScale(scale);
		}
	}

	for (auto node : nodes) {
		node->draw(ctx);
	}

	glBindVertexArray(0);

	return 0;
}

void Test6::processInput(float dt) {
	Engine::processInput(dt);
}

