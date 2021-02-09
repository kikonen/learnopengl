#include "SceneSetup1.h"

#include "MeshLoader.h"
#include "Terrain.h"
#include "InstancedNode.h"
#include "AsteroidBeltUpdater.h"
#include "MovingLightUpdater.h"
#include "NodePathUpdater.h"

#include "NodeType.h"
#include "TerrainGenerator.h"

SceneSetup1::SceneSetup1(const Assets& assets)
	: assets(assets)
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

void SceneSetup1::setupNodeSkybox(Scene* scene)
{
	SkyboxRenderer* skybox = new SkyboxRenderer(assets, "skybox");
	skybox->prepare();

	scene->skyboxRenderer = skybox;
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

void SceneSetup1::setupNodeDirectional(Scene* scene)
{
	Light* sun = scene->getDirLight();
	if (!sun) return;

	//sun->target = planet->getPos();

	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_LIGHT));
	type->light = true;

	MeshLoader loader(assets, "light");
	loader.defaultMaterial->kd = sun->specular;
	loader.overrideMaterials = true;
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(sun->pos);
	node->setScale(1.5f);
	scene->addNode(node);

	const float radius = 80.0f;
	const float speed = 20.f;
	glm::vec3 center = glm::vec3(0, 40, 0) + assets.groundOffset;
	if (planet) {
		center = planet->getPos();
	}
	node->updater = new MovingLightUpdater(assets, center, radius, speed, scene->getDirLight());
}

void SceneSetup1::setupNodeLightMoving(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_LIGHT));
	type->light = true;

	MeshLoader loader(assets, "light");
	loader.overrideMaterials = true;
	type->mesh = loader.load();

	glm::vec3 center = glm::vec3(0, 7, 0) + assets.groundOffset;

	for (auto light : scene->getPointLights()) {
		Node* node = new Node(type);
		node->setPos(light->pos);
		node->setScale(0.5f);
		scene->addNode(node);
		if (light == activeLight) {
			node->updater = new MovingLightUpdater(assets, center, 10.f, 2.f, light);
		}
	}

	for (auto light : scene->getSpotLights()) {
		Node* node = new Node(type);
		node->setPos(light->pos);
		node->setScale(0.5f);
		scene->addNode(node);
		if (light == activeLight) {
			node->updater = new MovingLightUpdater(assets, center, 10.f, 2.f, light);
		}
	}
}

void SceneSetup1::setupNodeZero(Scene* scene) {
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "water_ball");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(0, 0, 0) + assets.groundOffset);
	node->setScale(0.3f);
	scene->addNode(node);
}

void SceneSetup1::setupNodeWindow1(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	type->blend = true;
	type->renderBack = true;

	MeshLoader loader(assets, "window1");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(5, 10, -5) + assets.groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	scene->addNode(node);
}

void SceneSetup1::setupNodeWindow2(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	type->blend = true;
	type->renderBack = true;

	MeshLoader loader(assets, "window2");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(7, 10, -8) + assets.groundOffset);
	node->setRotation(glm::vec3(0, 180, 0));
	scene->addNode(node);
	//	selection.push_back(node);
}

void SceneSetup1::setupNodeStainedWindows(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	type->blend = true;
	type->renderBack = true;

	MeshLoader loader(assets, "window2");
	type->mesh = loader.load();

	for (int i = 0; i < 10; i++) {
		Node* node = new Node(type);
		node->setPos(glm::vec3(-10 + i * 2, 15, 10) + assets.groundOffset);
		node->setRotation(glm::vec3(0, 180, 0));
		scene->addNode(node);
	}
}

void SceneSetup1::setupNodeBrickwall(Scene* scene)
{
	NodeType* type1 = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	type1->renderBack = true;

	MeshLoader loader1(assets, "brickwall");
	type1->mesh = loader1.load();

	NodeType* type2 = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	type2->renderBack = true;

	MeshLoader loader2(assets, "brickwall2");
	type2->mesh = loader2.load();

	for (int i = 0; i < 5; i++) {
		Node* node = new Node(i % 2 == 0 ? type1 : type2);
		node->setPos(glm::vec3(-5 + i * 2, 5, 14) + assets.groundOffset);
		//node->setRotation(glm::vec3(0, 180, 0));
		scene->addNode(node);
	}
}

void SceneSetup1::setupNodeBrickwallBox(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	type->renderBack = true;

	MeshLoader loader(assets, "brickwall2");
	type->mesh = loader.load();

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
		Node* node = new Node(type);
		node->setPos(pos[i] * glm::vec3(scale, scale, scale) + glm::vec3(0, 95, 0) + assets.groundOffset);
		node->setScale(scale);
		node->setRotation(rot[i]);
		//node->skipShadow = true;
		scene->addNode(node);
	}
}

void SceneSetup1::setupNodeSpyro(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "spyro2");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(0, 30, 30) + assets.groundOffset);
	node->setScale(0.1f);
	scene->addNode(node);
}

void SceneSetup1::setupNodeBackpack(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "backpack", "/backpack/");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(0, 5, 5) + assets.groundOffset);
	node->setScale(1.5f);
	scene->addNode(node);
}

void SceneSetup1::setupNodeTeapot(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	type->renderBack = true;

	MeshLoader loader(assets, "smooth_teapot");
	loader.defaultMaterial->kd = glm::vec4(0.578f, 0.578f, 0.168f, 1.f);
	//loader.overrideMaterials = true;
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(-5, 20, -5) + assets.groundOffset);
	node->selected = true;
	scene->addNode(node);
}

void SceneSetup1::setupNodeCow(Scene* scene)
{
	//mesh->defaultShader = getShader(TEX_PLAIN, "_explode");
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "texture_cow");
	loader.defaultMaterial->kd = glm::vec4(0.160f, 0.578f, 0.168f, 1.f);
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(5, 20, -5) + assets.groundOffset);
	node->selected = true;
	scene->addNode(node);
}

void SceneSetup1::setupNodeBall(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "texture_ball");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(0, 8, 0) + assets.groundOffset);
	node->setScale(2.0f);
	scene->addNode(node);
}

void SceneSetup1::setupNodeCube4(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "texture_cube_4");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(-5, 20, 5) + assets.groundOffset);
	node->selected = true;
	scene->addNode(node);
}

void SceneSetup1::setupNodeCubes(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "texture_cube_3");
	type->mesh = loader.load();

	std::vector<glm::vec3> points = {
		glm::vec3(-5, 15, -5),
		glm::vec3(5, 15, -5),
		glm::vec3(-5, 15, 5),
		glm::vec3(5, 15, 5),
	};

	for (auto p : points) {
		Node* node = new Node(type);
		node->setPos(p + assets.groundOffset);
		scene->addNode(node);
	}
}

void SceneSetup1::setupNodeActive(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "texture_cube");
	type->mesh = loader.load();

	Node* active = new Node(type);
	active->updater = new NodePathUpdater(assets, 0);
	active->setPos(glm::vec3(0) + assets.groundOffset);
	scene->addNode(active);

	Node* node = new Node(type);
	node->setPos(glm::vec3(5, 20, 5) + assets.groundOffset);
	scene->addNode(node);
}

void SceneSetup1::setupNodeMountains(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "texture_mountains");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(0));
	//		node->setScale(0.01);
	scene->addNode(node);
}

void SceneSetup1::setupNodeWaterBall(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "water_ball");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(glm::vec3(0, 20, 0) + assets.groundOffset);
	//node->setScale(0.5f);
	scene->addNode(node);
}

void SceneSetup1::setupNodePlanet(Scene* scene)
{
	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "planet", "/planet/");
	type->mesh = loader.load();

	Node* node = new Node(type);
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
}

void SceneSetup1::setupNodeAsteroids(Scene* scene)
{
	glm::vec3 planetPos = glm::vec3(10, 100, 100);

	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "rock", "/rock/");
	type->mesh = loader.load();

	Node* node = new Node(type);
	glm::vec3 pos = planet ? planet->getPos() - glm::vec3(0, 50, 0) : glm::vec3(10, 50, 100) + assets.groundOffset;
	node->setPos(pos);
	scene->addNode(node);
}

void SceneSetup1::setupNodeAsteroidBelt(Scene* scene)
{
	auto loader = [this](Scene* scene) {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type->batchMode = false;

		MeshLoader loader(assets, "rock", "/rock/");
		type->mesh = loader.load();

		AsteroidBeltUpdater* updater = new AsteroidBeltUpdater(assets, planet);
		InstancedNode* node = new InstancedNode(type, updater);
		//node->selected = true;
		scene->addNode(node);
	};
	scene->addLoader(loader);
}

void SceneSetup1::setupSpriteFlare(Scene* scene)
{
	NodeType* type = Sprite::getNodeType(assets, "Skeleton_VH.PNG");

	glm::vec3 pos = glm::vec3(0, 5, 20) + assets.groundOffset;
	for (int i = 0; i < 1001; i++) {
		Sprite* sprite = new Sprite(type, glm::vec2(1.5, 3));
		sprite->setPos(pos + glm::vec3(0, 1.5, 0.1 * i));
		sprite->setRotation(glm::vec3(0, 0, 180));
		scene->addSprite(sprite);
	}
}

void SceneSetup1::setupTerrain(Scene* scene)
{
	Material* material = new Material("terrain");
	material->textureMode = GL_REPEAT;
	material->ns = 50;
	material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
	material->map_kd = "Grass Dark_VH.PNG";
	material->loadTextures(assets.texturesDir + "/");
	material->prepare();

	unsigned int textureIndex = 0;
	for (auto const& texture : material->textures) {
		texture->textureIndex = textureIndex;
		texture->unitID = GL_TEXTURE0 + textureIndex;
		textureIndex++;
	}

	Shader* shader = getShader(TEX_TERRAIN);
	shader->prepare();

	TerrainGenerator generator(assets);

	for (int x = 0; x < 2; x++) {
		for (int z = 0; z < 2; z++) {
			NodeType* type = new NodeType(NodeType::nextID(), shader);
			type->renderBack = true;
			type->mesh = generator.generateTerrain(material);

			Terrain* terrain = new Terrain(type, x, z);
			scene->addTerrain(terrain);
		}
	}
}

