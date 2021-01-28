#include "SceneSetup1.h"

#include "AsteroidBeltNode.h"


SceneSetup1::SceneSetup1(const Assets& assets, UBO& ubo)
	: assets(assets),
	ubo(ubo)
{
}

SceneSetup1::~SceneSetup1()
{
}

Shader* SceneSetup1::getShader(const std::string& name, const std::string& geometryType)
{
	return Shader::getShader(assets, name, geometryType);
}

void SceneSetup1::setup()
{
	scene = new Scene();

	setupUBOs();

	setupLightDirectional(scene);
	setupLightMoving(scene);

	setupNodeDirectional(scene);
	setupNodeLightMoving(scene);

	setupNodeZero(scene);

	setupNodeWaterBall(scene);

	setupNodeActive(scene);
	setupNodeCubes(scene);
	setupNodeCube4(scene);
	setupNodeBall(scene);
	setupNodeCow(scene);
	setupNodeTeapot(scene);
	//setupNodeBackpack(scene);

	setupNodeSpyro(scene);
	setupNodeWindow2(scene);
	setupNodeWindow1(scene);
	setupNodeStainedWindows(scene);

	setupNodeBrickwall(scene);

	setupNodeBrickwallBox(scene);
	//setupNodeMountains(scene);

	setupNodePlanet(scene);
	setupNodeAsteroids(scene);
	setupNodeAsteroidBelt(scene);

	setupNodeSkybox(scene);

	scene->stencilShader = getShader(TEX_STENCIL);
	scene->normalShader = getShader(TEX_NORMAL);
	scene->shadowShader = getShader(TEX_SIMPLE_DEPTH);
	scene->shadowDebugShader = getShader(TEX_DEBUG_DEPTH);
}

void SceneSetup1::process(RenderContext& ctx)
{
	moveActive(ctx);
	moveLight(ctx);
}

void SceneSetup1::bind(RenderContext& ctx)
{
	scene->bind(ctx);
}

void SceneSetup1::draw(RenderContext& ctx)
{
	if (sunNode) {
		sunNode->setPos(scene->dirLight->pos);
	}
	scene->draw(ctx);
}

void SceneSetup1::setupUBOs()
{
	// Matrices
	{
		glGenBuffers(1, &ubo.matrices);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.matrices);
		// projection + view
		int sz = sizeof(MatricesUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATRICES, ubo.matrices, 0, sz);
		ubo.matricesSize = sz;
	}
	// Data
	{
		glGenBuffers(1, &ubo.data);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.data);

		// cameraPos + time
		int sz = sizeof(DataUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_DATA, ubo.data, 0, sz);
		ubo.dataSize = sz;
	}
	// Lights
	{
		glGenBuffers(1, &ubo.lights);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.lights);
		// DirLight + PointLight + SpotLight
		int sz = sizeof(LightsUBO);
		int sz2 = sizeof(DirLightUBO);
		int sz3 = sizeof(PointLightUBO);
		int sz4 = sizeof(SpotLightUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_LIGHTS, ubo.lights, 0, sz);
		ubo.lightsSize = sz;
	}

	// materials
	{
		glGenBuffers(1, &ubo.materials);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo.materials);
		int sz = sizeof(MaterialsUBO);
		int sz2 = sizeof(MaterialUBO);
		glBufferData(GL_UNIFORM_BUFFER, sz, NULL, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, ubo.materials, 0, sz);
		ubo.materialsSize = sz;
	}
}

int SceneSetup1::setupNodeSkybox(Scene* scene)
{
	Skybox* skybox = new Skybox(assets, "skybox");
	skybox->prepare();

	scene->skybox = skybox;

	return 0;
}

void SceneSetup1::setupLightDirectional(Scene* scene)
{
	// sun
	Light* sun = new Light();
	sun->pos = glm::vec3(10, 100, 10) + scene->groundOffset;

	sun->dir = glm::vec3(-0.2f, -1.0f, -0.2f);
	sun->directional = true;

	sun->ambient = { 0.5f, 0.5f, 0.5f, 1.f };
	sun->diffuse = { 0.0f, 0.6f, 0.0f, 1.f };
	sun->specular = { 0.0f, 1.0f, 0.0f, 1.f };

	scene->dirLight = sun;
}

void SceneSetup1::setupLightMoving(Scene* scene)
{
	// light
	Light* light = new Light();
	light->pos = glm::vec3(10, -10, 10) + scene->groundOffset;

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
		scene->spotLights.push_back(light);
	}
	else {
		scene->pointLights.push_back(light);
	}
	activeLight = light;
}

int SceneSetup1::setupNodeDirectional(Scene* scene)
{
	Light* sun = scene->dirLight;

	// sun node
	if (!sun) {
		return -1;
	}
	ModelMesh* mesh = new ModelMesh("light");
	mesh->defaultShader = getShader(TEX_LIGHT);
	mesh->defaultMaterial->kd = sun->specular;

	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(sun->pos);
	node->setScale(1.5f);
	node->light = true;
	scene->nodes.push_back(node);
	sunNode = node;

	return 0;
}

int SceneSetup1::setupNodeLightMoving(Scene* scene)
{
	// light node
	ModelMesh* mesh = new ModelMesh("light");
	mesh->defaultShader = getShader(TEX_LIGHT);
	if (mesh->load(assets)) {
		return -1;
	}

	for (auto light : scene->pointLights) {
		Node* node = new Node(mesh);
		node->setPos(light->pos);
		node->setScale(0.5f);
		node->light = true;
		scene->nodes.push_back(node);
		if (light == activeLight) {
			activeLightNode = node;
		}
	}

	for (auto light : scene->spotLights) {
		Node* node = new Node(mesh);
		node->setPos(light->pos);
		node->setScale(0.5f);
		node->light = true;
		scene->nodes.push_back(node);
		if (light == activeLight) {
			activeLightNode = node;
		}
	}
	return 0;
}

void SceneSetup1::moveLight(RenderContext& ctx)
{
	float elapsed = glfwGetTime();

	//glm::vec3 planetPos = glm::vec3(10, 100, 100);
	const float radius = 10.0f;
	float posX = sin(elapsed / 2) * radius;
	float posZ = cos(elapsed / 2) * radius;

	glm::vec3 pos = glm::vec3(posX, -8, posZ) + scene->groundOffset;

	if (activeLight) {
		activeLight->pos = pos;
	}
	if (activeLightNode) {
		activeLightNode->setPos(pos);
	}
}

int SceneSetup1::setupNodeZero(Scene* scene) {
	ModelMesh* mesh = new ModelMesh("waterball");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, 0, 0) + scene->groundOffset);
	node->setScale(0.3f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeWindow1(Scene* scene)
{
	// window1
	ModelMesh* mesh = new ModelMesh("window1");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, -5, -5) + scene->groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	node->blend = true;
	node->flat = true;
	scene->nodes.push_back(node);
	scene->selection.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeWindow2(Scene* scene)
{
	// window2
	ModelMesh* mesh = new ModelMesh("window2");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(7, -5, -8) + scene->groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	node->blend = true;
	node->flat = true;
	scene->nodes.push_back(node);
	//	selection.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeStainedWindows(Scene* scene)
{
	// window2
	ModelMesh* mesh = new ModelMesh("window2");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	for (int i = 0; i < 10; i++) {
		Node* node = new Node(mesh);
		node->setPos(glm::vec3(-10 + i * 2, 0, 10) + scene->groundOffset);
		node->setRotation(glm::vec3(0, 180, 0));
		node->blend = true;
		node->flat = true;
		scene->nodes.push_back(node);
	}
	return 0;
}

int SceneSetup1::setupNodeBrickwall(Scene* scene)
{
	ModelMesh* mesh = new ModelMesh("brickwall");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	ModelMesh* mesh2 = new ModelMesh("brickwall2");
	mesh2->defaultShader = getShader(TEX_TEXTURE);
	if (mesh2->load(assets)) {
		return -1;
	}


	for (int i = 0; i < 5; i++) {
		Node* node = new Node(i % 2 == 0 ? mesh : mesh2);
		node->setPos(glm::vec3(-5 + i * 2, -8, 14) + scene->groundOffset);
		//node->setRotation(glm::vec3(0, 180, 0));
		node->flat = true;
		scene->nodes.push_back(node);
	}
	return 0;
}

int SceneSetup1::setupNodeBrickwallBox(Scene* scene)
{
	ModelMesh* mesh = new ModelMesh("brickwall2");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	glm::vec3 pos[] = {
		{0.0, 1.0, 0.0},
		{0.0, -1.0, .0},
		{1.0, 0.0, 0.0},
		{-1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 0.0, -1.0},
	};

	glm::vec3 rot[] = {
		{270, 0, 0},
		{90, 0, 0},
		{0, 90, 0},
		{0, 270, 0},
		{0, 0, 0},
		{0, 180, 0},
	};

	float scale = 40;
	for (int i = 0; i < 6; i++) {
		Node* node = new Node(mesh);
		node->setPos(pos[i] * glm::vec3(scale, scale, scale) + glm::vec3(0, 25, 0) + scene->groundOffset);
		node->setScale(scale);
		node->setRotation(rot[i]);
		node->flat = true;
		//node->skipShadow = true;
		scene->nodes.push_back(node);
	}
	return 0;
}

int SceneSetup1::setupNodeSpyro(Scene* scene)
{
	// spyro
	ModelMesh* mesh = new ModelMesh("spyro2");
	mesh->defaultShader = getShader(TEX_PLAIN);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, 20, 0) + scene->groundOffset);
	node->setScale(0.1f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeBackpack(Scene* scene)
{
	// backback
	ModelMesh* mesh = new ModelMesh("backpack", "/backpack/");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, -8, 0) + scene->groundOffset);
	node->setScale(1.5f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeTeapot(Scene* scene)
{
	// teapot
	ModelMesh* mesh = new ModelMesh("teapot");
	mesh->defaultShader = getShader(TEX_PLAIN);
	mesh->defaultMaterial->kd = glm::vec4(0.578f, 0.578f, 0.168f, 1.f);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(-5, 5, -5) + scene->groundOffset);
	scene->nodes.push_back(node);
	scene->selection.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeCow(Scene* scene)
{
	// cow
	ModelMesh* mesh = new ModelMesh("cow");
	mesh->defaultShader = getShader(TEX_PLAIN, "_explode");
	mesh->defaultMaterial->kd = glm::vec4(0.160f, 0.578f, 0.168f, 1.f);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, 5, -5) + scene->groundOffset);
	scene->nodes.push_back(node);
	scene->selection.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeBall(Scene* scene)
{
	// ball
	ModelMesh* mesh = new ModelMesh("texture_ball");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, -2, 0) + scene->groundOffset);
	node->setScale(2.0f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeCube4(Scene* scene)
{
	// cube 4
	ModelMesh* mesh = new ModelMesh("texture_cube_4");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(-5, 5, 5) + scene->groundOffset);
	scene->nodes.push_back(node);
	scene->selection.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeCubes(Scene* scene)
{
	// cubes
	ModelMesh* mesh = new ModelMesh("texture_cube_3");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
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
		node->setPos(p + scene->groundOffset);
		scene->nodes.push_back(node);
	}

	return 0;
}

int SceneSetup1::setupNodeActive(Scene* scene)
{
	// active
	ModelMesh* mesh = new ModelMesh("texture_cube");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	active = new Node(mesh);
	active->setPos(glm::vec3(0) + scene->groundOffset);
	scene->nodes.push_back(active);

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, 5, 5) + scene->groundOffset);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeMountains(Scene* scene)
{
	// mountains
	ModelMesh* mesh = new ModelMesh("texture_mountains");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0));
	//		node->setScale(0.01);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeWaterBall(Scene* scene)
{
	ModelMesh* mesh = new ModelMesh("light");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, 3, 0) + scene->groundOffset);
	//node->setScale(0.5f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodePlanet(Scene* scene)
{
	ModelMesh* mesh = new ModelMesh("planet", "/planet/");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(10, 100, 100) + scene->groundOffset);
	node->setScale(10);
	scene->nodes.push_back(node);

	{
		// light
		Light* light = new Light();
		light->pos = glm::vec3(13, 48, 100) + scene->groundOffset;

		// 160
		light->point = true;
		light->linear = 0.0027f;
		light->quadratic = 0.00028f;

		light->ambient = { 0.2f, 0.2f, 0.15f, 1.f };
		light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
		light->specular = { 1.0f, 1.0f, 0.9f, 1.f };

		scene->pointLights.push_back(light);
	}

	return 0;
}

int SceneSetup1::setupNodeAsteroids(Scene* scene)
{
	glm::vec3 planetPos = glm::vec3(10, 100, 100);

	ModelMesh* mesh = new ModelMesh("rock", "/rock/");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	if (mesh->load(assets)) {
		return -1;
	}

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(10, 50, 100) + scene->groundOffset);
	scene->nodes.push_back(node);

	return 0;
}

int SceneSetup1::setupNodeAsteroidBelt(Scene* scene)
{
	ModelMesh* mesh = new ModelMesh("rock", "/rock/");
	mesh->defaultShader = getShader(TEX_TEXTURE);
	mesh->load(assets);

	Node* node = new AsteroidBeltNode(mesh);
	scene->nodes.push_back(node);
//	scene->selection.push_back(node);
	return 0;
}

void SceneSetup1::moveActive(RenderContext& ctx)
{
	if (!active) {
		return;
	}

	float elapsed = glfwGetTime();

	if (true) {
		const float radius = 4.0f;
		float posX = sin(elapsed / 0.9f) * radius;
		float posY = sin(elapsed * 1.1f) * radius / 3.0f;
		float posZ = cos(elapsed) * radius / 2.0f;

		active->setPos(glm::vec3(posX, posY, posZ) + scene->groundOffset);
	}

	if (true) {
		const float radius = 2.0f;
		float rotX = elapsed * radius;
		float rotY = elapsed * radius * 1.1f;
		float rotZ = elapsed * radius * 1.2f;

		active->setRotation(glm::vec3(rotX, rotY, rotZ));
	}

	if (true) {
		const float radius = 2.0f;
		float scale = sin(elapsed / 4.0f) * radius;

		active->setScale(scale);
	}
}
