#include "SceneSetup1.h"

#include "AsteroidBeltNode.h"
#include "ModelMeshLoader.h"
#include "Terrain.h"


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
	scene = new Scene(assets);

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

	setupNodeSpyro(scene);
	setupNodeWindow2(scene);
	setupNodeWindow1(scene);
	setupNodeStainedWindows(scene);

	//setupNodeBackpack(scene);

	setupNodeBrickwall(scene);

	//setupNodeBrickwallBox(scene);
	//setupNodeMountains(scene);

	setupNodePlanet(scene);
	setupNodeAsteroids(scene);
	setupNodeAsteroidBelt(scene);

	setupTerrain(scene);
	setupNodeSkybox(scene);
}

void SceneSetup1::process(RenderContext& ctx)
{
	moveActive(ctx);
	moveLight(ctx);
	moveDirLight(ctx);
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
	SkyboxRenderer* skybox = new SkyboxRenderer(assets, "skybox");
	skybox->prepare();

	scene->skyboxRenderer = skybox;

	return 0;
}

void SceneSetup1::setupLightDirectional(Scene* scene)
{
	// sun
	Light* sun = new Light();
	sun->pos = glm::vec3(10, 100, 10) + assets.groundOffset;

	sun->dir = glm::vec3(-0.2f, -1.0f, -0.2f);
	sun->directional = true;

	sun->ambient = { 0.3f, 0.3f, 0.3f, 1.f };
	sun->diffuse = { 0.5f, 0.5f, 0.5f, 1.f };
	sun->specular = { 0.0f, 0.8f, 0.0f, 1.f };

	scene->dirLight = sun;
}

void SceneSetup1::setupLightMoving(Scene* scene)
{
	// light
	Light* light = new Light();
	light->pos = glm::vec3(10, -10, 10) + assets.groundOffset;

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

	ModelMeshLoader loader(getShader(TEX_LIGHT), "light");
	loader.defaultMaterial->kd = sun->specular;
	ModelMesh* mesh = loader.load();

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
	ModelMeshLoader loader(getShader(TEX_LIGHT), "light");
	ModelMesh* mesh = loader.load();

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

	glm::vec3 pos = glm::vec3(posX, -8, posZ) + assets.groundOffset;

	if (activeLight) {
		activeLight->pos = pos;
	}
	if (activeLightNode) {
		activeLightNode->setPos(pos);
	}
}

int SceneSetup1::setupNodeZero(Scene* scene) {
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "water_ball");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, 0, 0) + assets.groundOffset);
	node->setScale(0.3f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeWindow1(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "window1");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, -5, -5) + assets.groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	node->blend = true;
	node->flat = true;
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeWindow2(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "window2");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(7, -5, -8) + assets.groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	node->blend = true;
	node->flat = true;
	scene->nodes.push_back(node);
	//	selection.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeStainedWindows(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "window2");
	ModelMesh* mesh = loader.load();

	for (int i = 0; i < 10; i++) {
		Node* node = new Node(mesh);
		node->setPos(glm::vec3(-10 + i * 2, 0, 10) + assets.groundOffset);
		node->setRotation(glm::vec3(0, 180, 0));
		node->blend = true;
		node->flat = true;
		scene->nodes.push_back(node);
	}
	return 0;
}

int SceneSetup1::setupNodeBrickwall(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "brickwall");
	ModelMesh* mesh = loader.load();

	ModelMeshLoader loader2(getShader(TEX_TEXTURE), "brickwall2");
	ModelMesh* mesh2 = loader2.load();

	for (int i = 0; i < 5; i++) {
		Node* node = new Node(i % 2 == 0 ? mesh : mesh2);
		node->setPos(glm::vec3(-5 + i * 2, -8, 14) + assets.groundOffset);
		//node->setRotation(glm::vec3(0, 180, 0));
		node->flat = true;
		scene->nodes.push_back(node);
	}
	return 0;
}

int SceneSetup1::setupNodeBrickwallBox(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "brickwall2");
	ModelMesh* mesh = loader.load();

	glm::vec3 pos[] = {
//		{0.0, 1.0, 0.0},
		{0.0, -1.0, .0},
//		{1.0, 0.0, 0.0},
//		{-1.0, 0.0, 0.0},
//		{0.0, 0.0, 1.0},
//		{0.0, 0.0, -1.0},
	};

	glm::vec3 rot[] = {
//		{270, 0, 0},
		{90, 0, 0},
//		{0, 90, 0},
//		{0, 270, 0},
//		{0, 0, 0},
//		{0, 180, 0},
	};

	float scale = 100;
	for (int i = 0; i < 1; i++) {
		Node* node = new Node(mesh);
		node->setPos(pos[i] * glm::vec3(scale, scale, scale) + glm::vec3(0, 90, 0) + assets.groundOffset);
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
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "spyro2");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, 20, 0) + assets.groundOffset);
	node->setScale(0.1f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeBackpack(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "backpack", "/backpack/");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, -8, 0) + assets.groundOffset);
	node->setScale(1.5f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeTeapot(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "smooth_teapot");
	loader.defaultMaterial->kd = glm::vec4(0.578f, 0.578f, 0.168f, 1.f);
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(-5, 5, -5) + assets.groundOffset);
	node->selected = true;
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeCow(Scene* scene)
{
	//mesh->defaultShader = getShader(TEX_PLAIN, "_explode");
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "texture_cow");
	loader.defaultMaterial->kd = glm::vec4(0.160f, 0.578f, 0.168f, 1.f);
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, 5, -5) + assets.groundOffset);
	node->selected = true;
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeBall(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "texture_ball");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, -2, 0) + assets.groundOffset);
	node->setScale(2.0f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeCube4(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "texture_cube_4");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(-5, 5, 5) + assets.groundOffset);
	node->selected = true;
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeCubes(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "texture_cube_3");
	ModelMesh* mesh = loader.load();

	std::vector<glm::vec3> points = {
		glm::vec3(-5, 0, -5),
		glm::vec3(5, 0, -5),
		glm::vec3(-5, 0, 5),
		glm::vec3(5, 0, 5),
	};

	for (auto p : points) {
		Node* node = new Node(mesh);
		node->setPos(p + assets.groundOffset);
		scene->nodes.push_back(node);
	}

	return 0;
}

int SceneSetup1::setupNodeActive(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "texture_cube");
	ModelMesh* mesh = loader.load();

	active = new Node(mesh);
	active->setPos(glm::vec3(0) + assets.groundOffset);
	scene->nodes.push_back(active);

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(5, 5, 5) + assets.groundOffset);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeMountains(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "texture_mountains");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0));
	//		node->setScale(0.01);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeWaterBall(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "water_ball");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(0, 3, 0) + assets.groundOffset);
	//node->setScale(0.5f);
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupNodePlanet(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "planet", "/planet/");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(10, 100, 100) + assets.groundOffset);
	node->setScale(10);
	scene->nodes.push_back(node);

	{
		// light
		Light* light = new Light();
		light->pos = glm::vec3(13, 48, 100) + assets.groundOffset;

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

	ModelMeshLoader loader(getShader(TEX_TEXTURE), "rock", "/rock/");
	ModelMesh* mesh = loader.load();

	Node* node = new Node(mesh);
	node->setPos(glm::vec3(10, 50, 100) + assets.groundOffset);
	scene->nodes.push_back(node);

	return 0;
}

int SceneSetup1::setupNodeAsteroidBelt(Scene* scene)
{
	ModelMeshLoader loader(getShader(TEX_TEXTURE), "rock", "/rock/");
	ModelMesh* mesh = loader.load();

	Node* node = new AsteroidBeltNode(mesh);
	//node->selected = true;
	scene->nodes.push_back(node);
	return 0;
}

int SceneSetup1::setupTerrain(Scene* scene)
{
	Material* material = new Material("terrain", 0);
	material->textureMode = GL_REPEAT;
	material->map_kd = "Grass Dark_VH.PNG";
	material->loadTextures(assets.modelsDir + "/");
	material->prepare();

	unsigned int textureIndex = 0;
	for (auto const& texture : material->textures) {
		texture->textureIndex = textureIndex;
		texture->unitID = GL_TEXTURE0 + textureIndex;
		textureIndex++;
	}

	Shader* shader = getShader(TEX_TERRAIN);
	shader->prepare();

	for (int x = 0; x < 2; x++) {
		for (int z = 0; z < 2; z++) {
			Terrain* terrain = new Terrain(x, z, material, shader);
			scene->terrains.push_back(terrain);
		}
	}
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

		active->setPos(glm::vec3(posX, posY, posZ) + assets.groundOffset);
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

void SceneSetup1::moveDirLight(RenderContext& ctx)
{
	if (!ctx.dirLight) {
		return;
	}

	const float radius = 15.0f;
	float posX = sin(glfwGetTime() / 8) * radius;
	float posZ = cos(glfwGetTime() / 8) * radius;

	glm::vec3 lightPos = glm::vec3(posX, 20, posZ) + assets.groundOffset;

	ctx.dirLight->pos = lightPos;
	//ctx.dirLight->dir = glm::normalize(target - lightPos);
}
