#include "Test6.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ModelMesh.h"

//glm::vec3 groundOffset(0.f, 15.f, -15.f);
glm::vec3 groundOffset(0.f, 15.f, -40.f);

Test6::Test6() {
	title = "Test 5";
	//throttleFps = 0;
}

int Test6::onSetup() {
	// sun
	if (true) {
		sun = new Light();
		sun->pos = glm::vec3(10, 100, 10) + groundOffset;

		sun->dir = glm::vec3(-0.2f, -1.0f, -0.2f);
		sun->directional = true;

		sun->ambient = { 0.1f, 0.1f, 0.1f };
		sun->diffuse = { 0.8f, 0.8f, 0.8f };
		sun->specular = { 1.0f, 1.0f, 1.0f };
	}

	// light
	if (true) {
		Light* light = new Light();
		light->pos = glm::vec3(10, -10, 10) + groundOffset;

		// 160
		light->point = true;
		light->linear = 0.027f;
		light->quadratic = 0.0028f;

		light->spot = false;
		light->cutoffAngle = 12.5f;
		light->outerCutoffAngle = 25.f;
		light->dir = glm::vec3(0.01f, 1.0f, 0.01f);

		light->ambient = { 0.2f, 0.2f, 0.15f };
		light->diffuse = { 0.8f, 0.8f, 0.7f };
		light->specular = { 1.0f, 1.0f, 0.9f };

		if (light->spot) {
			spotLights.push_back(light);
		} else {
			pointLights.push_back(light);
		}
		activeLight = light;
	}

	// sun node
	if (true && sun) {
		ModelMesh* mesh = new ModelMesh(*this, "light", "light6");
		mesh->defaultMaterial->kd = sun->specular;
		mesh->overrideMaterials = true;
		mesh->useTexture = false;

		if (mesh->load()) {
			return -1;
		}
		mesh->prepare();

		Node* node = new Node(mesh, sun->pos);
		node->setScale(4.f);
		nodes.push_back(node);
		sunNode = node;
	}

	// light node
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "light", "light6");
		mesh->useTexture = false;
		if (mesh->load()) {
			return -1;
		}
		mesh->prepare();

		for (auto light : pointLights) {
			Node* node = new Node(mesh, light->pos);
			//node->setScale(0.5f);
			nodes.push_back(node);
			if (light == activeLight) {
				activeLightNode = node;
			}
		}

		for (auto light : spotLights) {
			Node* node = new Node(mesh, light->pos);
			//node->setScale(0.5f);
			nodes.push_back(node);
			if (light == activeLight) {
				activeLightNode = node;
			}
		}
	}

	// waterball
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "light", "test6");
		//mesh->useWireframe = true;
		if (mesh->load()) {
			return -1;
		}
		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(0, 3, 0) + groundOffset);
		//node->setScale(0.5f);
		nodes.push_back(node);
	}

	// mountains
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "texture_mountains", "test6");
		//mesh->debugColors = true;
		if (mesh->load()) {
			return -1;
		}
		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(0));
		//		node->setScale(0.01);
		nodes.push_back(node);
	}

	// active
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "texture_cube", "test6");
		//mesh->useTexture = false;
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		active = new Node(mesh, glm::vec3(0) + groundOffset);
		nodes.push_back(active);

		nodes.push_back(new Node(mesh, glm::vec3(5, 5, 5) + groundOffset));
	}

	// cubes
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "texture_cube_3", "test6");
		//mesh->useTexture = false;
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		nodes.push_back(new Node(mesh, glm::vec3(-5, 0, -5) + groundOffset));
		nodes.push_back(new Node(mesh, glm::vec3(5, 0, -5) + groundOffset));
		nodes.push_back(new Node(mesh, glm::vec3(-5, 0, 5) + groundOffset));
		nodes.push_back(new Node(mesh, glm::vec3(5, 0, 5) + groundOffset));
	}

	// cube 4
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "texture_cube_4", "test6");
		//mesh->useTexture = false;
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		nodes.push_back(new Node(mesh, glm::vec3(-5, 5, 5) + groundOffset));
	}

	// ball
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "texture_ball", "test6");
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(0, -2, 0) + groundOffset);
		node->setScale(2.0f);
		nodes.push_back(node);
	}

	// cow
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "cow", "test6");
		mesh->defaultMaterial->kd = glm::vec3(0.160, 0.578, 0.168);
		mesh->overrideMaterials = true;
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(5, 5, -5) + groundOffset);
		nodes.push_back(node);
	}

	// teapot
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "teapot", "test6");
		mesh->defaultMaterial->kd = glm::vec3(0.578, 0.578, 0.168);
		mesh->overrideMaterials = true;
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(-5, 5, -5) + groundOffset);
		nodes.push_back(node);
	}

	// spyro
	if (true) {
		ModelMesh* mesh = new ModelMesh(*this, "spyro2", "test6");
		//mesh->color = glm::vec3(0.560, 0.278, 0.568);
		//mesh->useMaterialColor = false;
		if (mesh->load()) {
			return -1;
		}

		mesh->prepare();

		Node* node = new Node(mesh, glm::vec3(0, 20, 0) + groundOffset);
		node->setScale(0.1f);
		nodes.push_back(node);
	}

	camera.setPos(glm::vec3(-8, 5, 10.f) + groundOffset);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return 0;
}

int Test6::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// https://cmichel.io/understanding-front-faces-winding-order-and-normals
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_DEPTH_TEST);

	int w = 0;
	int h = 0;
	glfwGetWindowSize(window, &w, &h);

	const glm::mat4& view = camera.getView();
	const glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)w / (float)h, 0.1f, 1000.0f);

	RenderContext ctx(*this, dt, view, projection, sun, pointLights, spotLights);
	//ctx.useWireframe = true;
	//ctx.useLight = false;

	//	mesh->setPos(glm::vec3(0, 0, -10.0f));

	if (active) {
		if (true) {
			const float radius = 4.0f;
			float posX = sin(accumulatedTime / 0.9f) * radius;
			float posY = sin(accumulatedTime * 1.1f) * radius / 3.0f;
			float posZ = cos(accumulatedTime) * radius / 2.0f;

			active->setPos(glm::vec3(posX, posY, posZ) + groundOffset);
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

	if (true) {
		const float radius = 10.0f;
		float posX = sin(accumulatedTime / 2) * radius;
		float posZ = cos(accumulatedTime / 2) * radius;

		glm::vec3 pos = glm::vec3(posX, -8, posZ) + groundOffset;

		if (activeLight) {
			activeLight->pos = pos;
		}
		if (activeLightNode) {
			activeLightNode->setPos(pos);
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

