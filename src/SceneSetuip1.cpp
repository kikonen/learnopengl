#include "SceneSetup1.h"

#include "MeshLoader.h"
#include "Terrain.h"
#include "InstancedNode.h"
#include "AsteroidBeltUpdater.h"
#include "MovingLightUpdater.h"
#include "NodePathUpdater.h"

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

	setupNodeBrickwall(scene);

	//setupNodeBrickwallBox(scene);
	//setupNodeMountains(scene);

	setupNodePlanet(scene);
	setupNodeAsteroids(scene);
	setupNodeAsteroidBelt(scene);

	setupSpriteFlare(scene);

	setupTerrain(scene);
	setupNodeSkybox(scene);

	setupNodeDirectional(scene);
	setupNodeLightMoving(scene);

	//setupNodeBackpack(scene);
}

void SceneSetup1::process(RenderContext& ctx)
{
}

void SceneSetup1::update(RenderContext& ctx)
{
	scene->update(ctx);
}

void SceneSetup1::bind(RenderContext& ctx)
{
	scene->bind(ctx);
}

void SceneSetup1::draw(RenderContext& ctx)
{
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
	sun->pos = glm::vec3(10, 40, 10) + assets.groundOffset;
	sun->target = glm::vec3(0.0f) + assets.groundOffset;

	sun->directional = true;

	sun->ambient = { 0.2f, 0.2f, 0.2f, 1.f };
	sun->diffuse = { 0.5f, 0.5f, 0.5f, 1.f };
	sun->specular = { 0.0f, 0.8f, 0.0f, 1.f };

	scene->addLight(sun);
}

void SceneSetup1::setupLightMoving(Scene* scene)
{
	// light
	Light* light = new Light();
	light->pos = glm::vec3(10, 5, 10) + assets.groundOffset;

	// 160
	light->point = true;
	light->linear = 0.027f;
	light->quadratic = 0.0028f;

	light->spot = false;
	light->cutoffAngle = 12.5f;
	light->outerCutoffAngle = 25.f;

	light->target = glm::vec3(0.0f) + assets.groundOffset;

	light->ambient = { 0.2f, 0.2f, 0.15f, 1.f };
	light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
	light->specular = { 1.0f, 1.0f, 0.9f, 1.f };

	scene->addLight(light);

	activeLight = light;
}

int SceneSetup1::setupNodeDirectional(Scene* scene)
{
	Light* sun = scene->getDirLight();
	if (!sun) return -1;

	//sun->target = planet->getPos();

	MeshLoader loader(getShader(TEX_LIGHT), "light");
	loader.defaultMaterial->kd = sun->specular;
	loader.overrideMaterials = true;
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(sun->pos);
	node->setScale(1.5f);
	node->light = true;
	scene->addNode(node);

	const float radius = 80.0f;
	const float speed = 20.f;
	glm::vec3 center = glm::vec3(0, 40, 0) + assets.groundOffset;
	if (planet) {
		center = planet->getPos();
	}
	node->updater = new MovingLightUpdater(assets, center, radius, speed, scene->getDirLight());

	return 0;
}

int SceneSetup1::setupNodeLightMoving(Scene* scene)
{
	MeshLoader loader(getShader(TEX_LIGHT), "light");
	loader.overrideMaterials = true;
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	glm::vec3 center = glm::vec3(0, 7, 0) + assets.groundOffset;

	for (auto light : scene->getPointLights()) {
		Node* node = new Node(objectID, mesh);
		node->setPos(light->pos);
		node->setScale(0.5f);
		node->light = true;
		scene->addNode(node);
		if (light == activeLight) {
			node->updater = new MovingLightUpdater(assets, center, 10.f, 2.f, light);
		}
	}

	for (auto light : scene->getSpotLights()) {
		Node* node = new Node(objectID, mesh);
		node->setPos(light->pos);
		node->setScale(0.5f);
		node->light = true;
		scene->addNode(node);
		if (light == activeLight) {
			node->updater = new MovingLightUpdater(assets, center, 10.f, 2.f, light);
		}
	}
	return 0;
}

int SceneSetup1::setupNodeZero(Scene* scene) {
	MeshLoader loader(getShader(TEX_TEXTURE), "water_ball");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(0, 0, 0) + assets.groundOffset);
	node->setScale(0.3f);
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeWindow1(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "window1");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(5, 10, -5) + assets.groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	node->blend = true;
	node->renderBack = true;
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeWindow2(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "window2");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(7, 10, -8) + assets.groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	node->blend = true;
	node->renderBack = true;
	scene->addNode(node);
	//	selection.push_back(node);
	return 0;
}

int SceneSetup1::setupNodeStainedWindows(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "window2");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	for (int i = 0; i < 10; i++) {
		Node* node = new Node(objectID, mesh);
		node->setPos(glm::vec3(-10 + i * 2, 15, 10) + assets.groundOffset);
		node->setRotation(glm::vec3(0, 180, 0));
		node->blend = true;
		node->renderBack = true;
		scene->addNode(node);
	}
	return 0;
}

int SceneSetup1::setupNodeBrickwall(Scene* scene)
{
	MeshLoader loader1(getShader(TEX_TEXTURE), "brickwall");
	Mesh* mesh1 = loader1.load();

	const int objectID1 = Node::nextID();
	const int objectID2 = Node::nextID();

	MeshLoader loader2(getShader(TEX_TEXTURE), "brickwall2");
	Mesh* mesh2 = loader2.load();

	for (int i = 0; i < 5; i++) {
		Node* node = new Node(
			i % 2 == 0 ? objectID1 : objectID1,
			i % 2 == 0 ? mesh1 : mesh2);
		node->setPos(glm::vec3(-5 + i * 2, 5, 14) + assets.groundOffset);
		//node->setRotation(glm::vec3(0, 180, 0));
		node->renderBack = true;
		scene->addNode(node);
	}
	return 0;
}

int SceneSetup1::setupNodeBrickwallBox(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "brickwall2");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

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
		Node* node = new Node(objectID, mesh);
		node->setPos(pos[i] * glm::vec3(scale, scale, scale) + glm::vec3(0, 95, 0) + assets.groundOffset);
		node->setScale(scale);
		node->setRotation(rot[i]);
		node->renderBack = true;
		//node->skipShadow = true;
		scene->addNode(node);
	}
	return 0;
}

int SceneSetup1::setupNodeSpyro(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "spyro2");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(0, 30, 30) + assets.groundOffset);
	node->setScale(0.1f);
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeBackpack(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "backpack", "/backpack/");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(0, 5, 5) + assets.groundOffset);
	node->setScale(1.5f);
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeTeapot(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "smooth_teapot");
	loader.defaultMaterial->kd = glm::vec4(0.578f, 0.578f, 0.168f, 1.f);
	//loader.overrideMaterials = true;
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(-5, 20, -5) + assets.groundOffset);
	node->renderBack = true;
	node->selected = true;
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeCow(Scene* scene)
{
	//mesh->defaultShader = getShader(TEX_PLAIN, "_explode");
	MeshLoader loader(getShader(TEX_TEXTURE), "texture_cow");
	loader.defaultMaterial->kd = glm::vec4(0.160f, 0.578f, 0.168f, 1.f);
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(5, 20, -5) + assets.groundOffset);
	node->selected = true;
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeBall(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "texture_ball");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(0, 8, 0) + assets.groundOffset);
	node->setScale(2.0f);
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeCube4(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "texture_cube_4");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(-5, 20, 5) + assets.groundOffset);
	node->selected = true;
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeCubes(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "texture_cube_3");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	std::vector<glm::vec3> points = {
		glm::vec3(-5, 15, -5),
		glm::vec3(5, 15, -5),
		glm::vec3(-5, 15, 5),
		glm::vec3(5, 15, 5),
	};

	for (auto p : points) {
		Node* node = new Node(objectID, mesh);
		node->setPos(p + assets.groundOffset);
		scene->addNode(node);
	}

	return 0;
}

int SceneSetup1::setupNodeActive(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "texture_cube");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* active = new Node(objectID, mesh);
	active->updater = new NodePathUpdater(assets, 0);
	active->setPos(glm::vec3(0) + assets.groundOffset);
	scene->addNode(active);

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(5, 20, 5) + assets.groundOffset);
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeMountains(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "texture_mountains");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(0));
	//		node->setScale(0.01);
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodeWaterBall(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "water_ball");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	node->setPos(glm::vec3(0, 20, 0) + assets.groundOffset);
	//node->setScale(0.5f);
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupNodePlanet(Scene* scene)
{
	auto loader = [this](Scene* scene) {
		MeshLoader loader(getShader(TEX_TEXTURE), "planet", "/planet/");
		Mesh* mesh = loader.load();

		const int objectID = Node::nextID();

		Node* node = new Node(objectID, mesh);
		node->setPos(glm::vec3(10, 100, 100) + assets.groundOffset);
		node->setScale(10);

		scene->addNode(node);

		planet = node;

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

			scene->addLight(light);
		}
	};
	loader(scene);

	return 0;
}

int SceneSetup1::setupNodeAsteroids(Scene* scene)
{
	glm::vec3 planetPos = glm::vec3(10, 100, 100);

	MeshLoader loader(getShader(TEX_TEXTURE), "rock", "/rock/");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	Node* node = new Node(objectID, mesh);
	glm::vec3 pos = planet ? planet->getPos() - glm::vec3(0, 50, 0) : glm::vec3(10, 50, 100) + assets.groundOffset;
	node->setPos(pos);
	scene->addNode(node);

	return 0;
}

int SceneSetup1::setupNodeAsteroidBelt(Scene* scene)
{
	MeshLoader loader(getShader(TEX_TEXTURE), "rock", "/rock/");
	Mesh* mesh = loader.load();

	const int objectID = Node::nextID();

	AsteroidBeltUpdater* updater = new AsteroidBeltUpdater(assets, planet);
	InstancedNode* node = new InstancedNode(objectID, mesh, updater);
	//node->selected = true;
	scene->addNode(node);
	return 0;
}

int SceneSetup1::setupSpriteFlare(Scene* scene)
{
	Material* material = new Material("flare");
	material->ns = 100;
	material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
	material->map_kd = "Skeleton_VH.PNG";
	material->loadTextures(assets.spritesDir + "/");
	material->prepare();

	const int objectID = Node::nextID();

	Sprite* sprite = new Sprite(objectID, glm::vec2(2, 2), material);
	sprite->setPos(glm::vec3(0, 5, 20) + assets.groundOffset);
	scene->addSprite(sprite);

	return 0;
}

int SceneSetup1::setupTerrain(Scene* scene)
{
	Material* material = new Material("terrain");
	material->textureMode = GL_REPEAT;
	material->ns = 50;
	material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
	material->map_kd = "Grass Dark_VH.PNG";
	material->loadTextures(assets.texturesDir + "/");
	material->prepare();

	const int objectID = Node::nextID();

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
			Terrain* terrain = new Terrain(objectID, x, z, material, shader);
			scene->addTerrain(terrain);
		}
	}
	return 0;
}

