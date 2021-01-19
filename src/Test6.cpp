#include "Test6.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ModelMesh.h"
#include "AsteroidBeltNode.h"

//glm::vec3 groundOffset(0.f, 15.f, -15.f);
glm::vec3 groundOffset(0.f, 15.f, -40.f);

Test6::Test6() {
	title = "Test 6";
	//throttleFps = 0;
	glfwWindowHint(GLFW_SAMPLES, 4);
}

int Test6::onSetup() {
	setupLightSun();
	setupLightMoving();

	setupNodeSun();
	setupNodeLightMoving();
	setupNodeWaterBall();

	setupNodeActive();
	setupNodeCubes();
	setupNodeCube4();
	setupNodeBall();
	setupNodeCow();
	setupNodeTeapot();
	//setupNodeBackpack();

 	setupNodeSpyro();
	setupNodeWindow2();
	setupNodeWindow1();
	setupNodeStainedWindows();

	setupNodeMountains();

	setupNodePlanet();
	setupNodeAsteroids();
	setupNodeAsteroidBelt();

	setupNodeSkybox();

	camera.setPos(glm::vec3(-8, 5, 10.f) + groundOffset);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_MULTISAMPLE);

	stencilShader = getShader("test6_stencil");
	normalShader = getShader("test6_normal");

	setupUBOs();

	return 0;
}

int Test6::setupNodeSkybox()
{
	skybox = new Skybox(assets, "skybox");
	skybox->prepare();

	return 0;
}

int Test6::setupNodeWindow1()
{
	// window1
	ModelMesh* mesh = new ModelMesh("window1");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, -5, -5) + groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	node->blend = true;
	nodes.push_back(node);
	selection.push_back(node);
	return 0;
}

int Test6::setupNodeWindow2()
{
	// window2
	ModelMesh* mesh = new ModelMesh("window2");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(7, -5, -8) + groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	node->blend = true;
	nodes.push_back(node);
	//	selection.push_back(node);
	return 0;
}

int Test6::setupNodeStainedWindows()
{
	// window2
	ModelMesh* mesh = new ModelMesh("window2");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	for (int i = 0; i < 10; i++) {
		Node* node = new Node(mesh);
		node->setPos(glm::vec3(-10 + i * 2, 0, 10) + groundOffset);
		node->setRotation(glm::vec3(0, 180, 0));
		node->blend = true;
		nodes.push_back(node);
	}
	return 0;
}

int Test6::setupNodeSpyro()
{
	// spyro
	ModelMesh* mesh = new ModelMesh("spyro2");
	mesh->defaultShader = getShader("test6");
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, 20, 0) + groundOffset);
	node->setScale(0.1f);
	nodes.push_back(node);
	return 0;
}

int Test6::setupNodeBackpack()
{
	// backback
	ModelMesh* mesh = new ModelMesh("backpack", "/backpack/");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, -8, 0) + groundOffset);
	node->setScale(1.5f);
	nodes.push_back(node);
	return 0;
}

int Test6::setupNodeTeapot()
{
	// teapot
	ModelMesh* mesh = new ModelMesh("teapot");
	mesh->defaultShader = getShader("test6");
	mesh->defaultMaterial->kd = glm::vec4(0.578f, 0.578f, 0.168f, 1.f);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(-5, 5, -5) + groundOffset);
	nodes.push_back(node);
	selection.push_back(node);
	return 0;
}

int Test6::setupNodeCow()
{
	// cow
	ModelMesh* mesh = new ModelMesh("cow");
	mesh->defaultShader = getShader("test6", "_explode");
	mesh->defaultMaterial->kd = glm::vec4(0.160f, 0.578f, 0.168f, 1.f);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, 5, -5) + groundOffset);
	nodes.push_back(node);
	selection.push_back(node);
	return 0;
}

int Test6::setupNodeBall()
{
	// ball
	ModelMesh* mesh = new ModelMesh("texture_ball");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, -2, 0) + groundOffset);
	node->setScale(2.0f);
	nodes.push_back(node);
	return 0;
}

int Test6::setupNodeCube4()
{
	// cube 4
	ModelMesh* mesh = new ModelMesh("texture_cube_4");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(-5, 5, 5) + groundOffset);
	nodes.push_back(node);
	selection.push_back(node);
	return 0;
}

int Test6::setupNodeCubes()
{
	// cubes
	ModelMesh* mesh = new ModelMesh("texture_cube_3");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	std::vector<glm::vec3> points = {
		glm::vec3(-5, 0, -5),
		glm::vec3(5, 0, -5),
		glm::vec3(-5, 0, 5),
		glm::vec3(5, 0, 5),
	};

	for (auto p : points) {
		Node* node = new Node(mesh);
		node->setPos(p + groundOffset);
		nodes.push_back(node);
	}

	return 0;
}

int Test6::setupNodeActive()
{
	// active
	ModelMesh* mesh = new ModelMesh("texture_cube");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	active = new Node(mesh);
	active->setPos(glm::vec3(0) + groundOffset);
	nodes.push_back(active);

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, 5, 5) + groundOffset);
	nodes.push_back(node);
	return 0;
}

int Test6::setupNodeMountains()
{
	// mountains
	ModelMesh* mesh = new ModelMesh("texture_mountains");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0));
	//		node->setScale(0.01);
	nodes.push_back(node);
	return 0;
}

int Test6::setupNodeWaterBall()
{
	ModelMesh* mesh = new ModelMesh("light");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, 3, 0) + groundOffset);
	//node->setScale(0.5f);
	nodes.push_back(node);
	return 0;
}

int Test6::setupNodePlanet()
{
	ModelMesh* mesh = new ModelMesh("planet", "/planet/");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(10, 100, 100) + groundOffset);
	node->setScale(10);
	nodes.push_back(node);

	{
		// light
		Light* light = new Light();
		light->pos = glm::vec3(13, 48, 100) + groundOffset;

		// 160
		light->point = true;
		light->linear = 0.0027f;
		light->quadratic = 0.00028f;

		light->ambient = { 0.2f, 0.2f, 0.15f, 1.f };
		light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
		light->specular = { 1.0f, 1.0f, 0.9f, 1.f };

		pointLights.push_back(light);
	}

	return 0;
}

int Test6::setupNodeAsteroids()
{
	glm::vec3 planetPos = glm::vec3(10, 100, 100);

	ModelMesh* mesh = new ModelMesh("rock", "/rock/");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(10, 50, 100) + groundOffset);
	nodes.push_back(node);

	return 0;
}

int Test6::setupNodeAsteroidBelt()
{
	ModelMesh* mesh = new ModelMesh("rock", "/rock/");
	mesh->defaultShader = getShader("test6" + TEX_TEXTURE);
	mesh->load(*this);

	Node* node = new AsteroidBeltNode(mesh);
	nodes.push_back(node);
	return 0;
}

int Test6::setupNodeLightMoving()
{
	// light node
	ModelMesh* mesh = new ModelMesh("light");
	mesh->defaultShader = getShader("light6");
	if (mesh->load(*this)) {
		return -1;
	}

	for (auto light : pointLights) {
		Node* node = new Node(mesh);
		node->setPos(light->pos);
		//node->setScale(0.5f);
		nodes.push_back(node);
		if (light == activeLight) {
			activeLightNode = node;
		}
	}

	for (auto light : spotLights) {
		Node* node = new Node(mesh);
		node->setPos(light->pos);
		//node->setScale(0.5f);
		nodes.push_back(node);
		if (light == activeLight) {
			activeLightNode = node;
		}
	}
	return 0;
}

int Test6::setupNodeSun()
{
	// sun node
	if (!sun) {
		return -1;
	}
	ModelMesh* mesh = new ModelMesh("light");
	mesh->defaultShader = getShader("light6");
	mesh->defaultMaterial->kd = sun->specular;

	if (mesh->load(*this)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(sun->pos);
	node->setScale(4.f);
	nodes.push_back(node);
	sunNode = node;

	return 0;
}

void Test6::setupLightMoving()
{
	// light
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

	light->ambient = { 0.2f, 0.2f, 0.15f, 1.f };
	light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
	light->specular = { 1.0f, 1.0f, 0.9f, 1.f };

	if (light->spot) {
		spotLights.push_back(light);
	}
	else {
		pointLights.push_back(light);
	}
	activeLight = light;
}

void Test6::setupLightSun()
{
	// sun
	sun = new Light();
	sun->pos = glm::vec3(10, 100, 10) + groundOffset;

	sun->dir = glm::vec3(-0.2f, -1.0f, -0.2f);
	sun->directional = true;

	sun->ambient = { 0.1f, 0.1f, 0.1f, 1.f };
	sun->diffuse = { 0.8f, 0.8f, 0.8f, 1.f };
	sun->specular = { 1.0f, 1.0f, 1.0f, 1.f };
}

void Test6::setupUBOs()
{
	// Matrices
	{
		glGenBuffers(1, &ubo.matrices);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.matrices);
		// projection + view
		int sz = UBO_MAT_SIZE * 2;
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATRICES, ubo.matrices, 0, sz);
	}
	// Data
	{
		glGenBuffers(1, &ubo.data);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.data);
		// ??
		int sz = UBO_VEC_SIZE;
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_DATA, ubo.data, 0, sz);
	}
	// Lights
	{
		glGenBuffers(1, &ubo.lights);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.lights);
		// DirLight + PointLight + SpotLight
		int s2 = SZ_DIR_LIGHT_UBO;
		int sz = SZ_DIR_LIGHT_UBO;// +SZ_POINT_LIGHT_UBO + SZ_SPOT_LIGHT_UBO;
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_LIGHTS, ubo.lights, 0, sz);
	}
}

int Test6::onRender(float dt) {
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//showNormals = true;

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

	RenderContext ctx(*this, dt, view, projection, skybox->textureID, sun, pointLights, spotLights);
	//ctx.useWireframe = true;
	//ctx.useLight = false;

	ctx.bindGlobal();

	moveActive();
	moveLight();

	// NOTE KI OpenGL does NOT like interleaved draw and prepare
	for (auto node : nodes) {
		node->prepare(nullptr);
		node->prepare(stencilShader);
		if (showNormals) {
			node->prepare(normalShader);
		}
	}

	drawSelected(ctx);
	drawNodes(ctx);
	drawSelectedStencil(ctx);

	drawNormals(ctx);

	return 0;
}

void Test6::drawNormals(RenderContext& ctx)
{
	if (!showNormals) {
		return;
	}

	for (auto node : nodes) {
		node->bind(ctx, normalShader);
		node->draw(ctx);
	}
}

// draw all non selected nodes
void Test6::drawNodes(RenderContext& ctx)
{
	glStencilMask(0x00);
	std::vector<Node*> blendedNodes;
	for (auto node : nodes) {
		if (std::find(selection.begin(), selection.end(), node) != selection.end()) {
			continue;
		}
		if (node->blend) {
			blendedNodes.push_back(node);
		}
		else {
			node->bind(ctx, nullptr);
			node->draw(ctx);
		}
	}

	if (skybox) {
		skybox->draw(ctx);
	}
	renderBlended(blendedNodes, ctx);
}

// draw all selected nodes for stencil
void Test6::drawSelected(RenderContext& ctx)
{
	if (selection.empty()) {
		return;
	}

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	std::vector<Node*> blendedNodes;
	for (auto node : selection) {
		if (node->blend) {
			blendedNodes.push_back(node);
		}
		else {
			node->bind(ctx, nullptr);
			node->draw(ctx);
		}
	}

	renderBlended(blendedNodes, ctx);
}

// draw all selected nodes with stencil
void Test6::drawSelectedStencil(RenderContext& ctx)
{
	if (selection.empty()) {
		return;
	}

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	for (auto node : selection) {
		float scale = node->getScale();
		node->setScale(scale * 1.02);
		node->bind(ctx, stencilShader);
		node->draw(ctx);
		node->setScale(scale);
	}

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glEnable(GL_DEPTH_TEST);
}

void Test6::moveLight()
{
	//glm::vec3 planetPos = glm::vec3(10, 100, 100);
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

void Test6::moveActive()
{
	if (!active) {
		return;
	}

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

void Test6::processInput(float dt) {
	Engine::processInput(dt);
}

void Test6::renderBlended(std::vector<Node*>& nodes, RenderContext& ctx)
{
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// TODO KI discards nodes if *same* distance
	std::map<float, Node*> sorted;
	for (auto node : nodes) {
		float distance = glm::length(camera.getPos() - node->getPos());
		sorted[distance] = node;
	}

	for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
		Node* node = it->second;
		node->bind(ctx, nullptr);
		node->draw(ctx);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}


