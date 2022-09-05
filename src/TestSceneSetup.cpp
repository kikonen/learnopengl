#include "TestSceneSetup.h"

#include "asset/MeshLoader.h"
#include "asset/PlainTexture.h"

#include "model/Sprite.h"
#include "model/Terrain.h"
#include "model/Billboard.h"
#include "model/Water.h"
#include "model/InstancedNode.h"

#include "controller/CameraController.h"
#include "controller/AsteroidBeltController.h"
#include "controller/MovingLightController.h"
#include "controller/NodePathController.h"

#include "scene/NodeType.h"
#include "scene/TerrainGenerator.h"


TestSceneSetup::TestSceneSetup(const Assets& assets)
	: assets(assets), loader(assets)
{
}

TestSceneSetup::~TestSceneSetup()
{
}

void TestSceneSetup::setup(std::shared_ptr<Scene> scene)
{
	NodeType::setBaseID(10000);

	this->scene = scene;
	loader.scene = scene;

	setupCamera();

	setupNodeSkybox();

	setupNodeBrickCube();

	setupNodeCubes();
	setupNodeCube4();

	setupNodeActive();

	setupNodePlanet();
	setupNodeAsteroid();
	setupNodeAsteroidBelt();

	setupSpriteSkeleton();

	setupTerrain();

	setupWaterBottom();
	setupWaterSurface();

	//setupEffectExplosion();

	//setupViewport1();

	setupLightDirectional();
	setupLightMoving();
}

void TestSceneSetup::setupCamera()
{
	glm::vec3 pos = glm::vec3(-8, 5, 10.f) + assets.groundOffset;
	glm::vec3 front = glm::vec3(0, 0, -1);
	glm::vec3 up = glm::vec3(0, 1, 0);

	auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "player");
	type->mesh = loader.load();

	Node* node = new Node(type);
	{
		node->setPos(pos);
		node->setScale(0.8f);
		node->camera = std::make_unique<Camera>(pos, front, up);
		node->controller = new CameraController(assets);

		ParticleDefinition pd;
		node->particleGenerator = std::make_unique<ParticleGenerator>(assets, pd);
	}

	scene->registry.addNode(node);
}

void TestSceneSetup::setupNodeSkybox()
{
	//addLoader([this]() {
	//});
	SkyboxRenderer* skybox = new SkyboxRenderer(assets, "skybox");
	skybox->prepare(scene->shaders);

	scene->skyboxRenderer.reset(skybox);
}

void TestSceneSetup::setupLightDirectional()
{
	// sun
	Light* light = new Light();
	{
		light->setPos(glm::vec3(10, 40, 10) + assets.groundOffset);
		light->setTarget(glm::vec3(0.0f) + assets.groundOffset);

		light->directional = true;

		light->ambient = { 0.4f, 0.4f, 0.4f, 1.f };
		light->diffuse = { 0.4f, 0.4f, 0.4f, 1.f };
		light->specular = { 0.0f, 0.7f, 0.0f, 1.f };
	}

	loader.addLoader([this, light]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_LIGHT));
		type->light = true;
		type->noShadow = true;

		MeshLoader loader(assets, "light");
		loader.defaultMaterial->kd = light->specular;
		loader.overrideMaterials = true;
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(light->getPos());
		node->setScale(1.5f);
		node->light = light;

		{
			const float radius = 80.0f;
			const float speed = 20.f;
			glm::vec3 center = glm::vec3(0, 40, 0) + assets.groundOffset;

			Node* planet = getPlanet();
			if (planet) {
				center = planet->getPos();
			}

			node->controller = new MovingLightController(assets, center, radius, speed, node);
		}

		scene->registry.addNode(node);
	});
}

void TestSceneSetup::setupLightMoving()
{
	//Light* active = nullptr;
	std::vector<Light*> lights;

	float radius = 10.f;

	for (int x = 0; x < 2; x ++) {
		for (int z = 0; z < 2; z++) {
			Light* light = new Light();

			glm::vec3 center = glm::vec3(0 + x * radius * 3, 7 + x + z, z * radius * 3) + assets.groundOffset;
			//light->pos = glm::vec3(10, 5, 10) + assets.groundOffset;
			light->setPos(center);

			// 160
			light->point = true;
			light->linear = 0.14f;
			light->quadratic = 0.07f;

			light->spot = false;
			light->cutoffAngle = 12.5f;
			light->outerCutoffAngle = 25.f;

			light->setTarget(glm::vec3(0.0f) + assets.groundOffset);

			light->ambient = { 0.4f, 0.4f, 0.2f, 1.f };
			light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
			light->specular = { 1.0f, 1.0f, 0.9f, 1.f };

			lights.push_back(light);
		}
	}

	loader.addLoader([this, lights]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_LIGHT));
		type->light = true;
		type->noShadow = true;
		//type->wireframe = true;

		MeshLoader loader(assets, "light");
		//loader.overrideMaterials = true;
		type->mesh = loader.load();

		for (auto light : lights) {
			Node* node = new Node(type);
			node->setPos(light->getPos());
			node->setScale(0.5f);
			node->light = light;

			{
				glm::vec3 center = light->getPos();
				node->controller = new MovingLightController(assets, center, 10.f, 2.f, node);
			}

			scene->registry.addNode(node);
		}
	});
}

void TestSceneSetup::setupNodeBrickwallBox()
{
	loader.addLoader([this]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
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
			scene->registry.addNode(node);
		}
	});
}

void TestSceneSetup::setupNodeCube4()
{
	loader.addLoader([this]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "texture_cube_4");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(-5, 20, 5) + assets.groundOffset);
		node->selected = true;
		scene->registry.addNode(node);
	});
}

void TestSceneSetup::setupNodeCubes()
{
	loader.addLoader([this]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
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
			scene->registry.addNode(node);
		}
	});
}

void TestSceneSetup::setupNodeActive()
{
	loader.addLoader([this]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "texture_cube");
		type->mesh = loader.load();

		Node* active = new Node(type);
		active->controller = new NodePathController(assets, 0);
		active->setPos(glm::vec3(0) + assets.groundOffset);
		scene->registry.addNode(active);
	});
}

void TestSceneSetup::setupNodeBrickCube()
{
	loader.addLoader([this]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "texture_cube");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(5, 20, 5) + assets.groundOffset);
		scene->registry.addNode(node);
	});
}

void TestSceneSetup::setupNodePlanet()
{
	std::lock_guard<std::mutex> lock(planet_lock);

	planetFutureIndex = loader.addLoader([this]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "planet", "/planet/");
		type->mesh = loader.load();
		type->modifyMaterials([](Material& m) { m.fogRatio = 0; });

		Node* node = new Node(type);
		node->setPos(glm::vec3(10, 100, 100) + assets.groundOffset);
		node->setScale(10);

		scene->registry.addNode(node);
		setPlanet(node);
	});

	loader.addLoader([this]() {
		Node* planet = getPlanet();

		Light* light = new Light();
		{
			glm::vec3 pos = planet ? planet->getPos() - glm::vec3(0, 40, 0) : glm::vec3(0, 40, 0);
			light->setPos(pos);

			// 325 = 0.014	0.0007
			light->point = true;
			light->linear = 0.014f;
			light->quadratic = 0.0007f;

			light->ambient = { 0.4f, 0.4f, 0.2f, 1.f };
			light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
			light->specular = { 1.0f, 1.0f, 0.9f, 1.f };
		}

		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_LIGHT));
		{
			type->light = true;
			type->noShadow = true;

			MeshLoader loader(assets, "light");
			loader.overrideMaterials = true;
			type->mesh = loader.load();
		}

		Node* node = new Node(type);
		node->setPos(light->getPos());
		node->setScale(0.5f);

		node->light = light;

		{
			glm::vec3 center = light->getPos();
			node->controller = new MovingLightController(assets, center, 4.f, 2.f, node);
		}

		scene->registry.addNode(node);
	});
}

void TestSceneSetup::setupNodeAsteroid()
{
	loader.addLoader([this]() {
		glm::vec3 planetPos = glm::vec3(10, 100, 100);

		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
		{
			MeshLoader loader(assets, "rock", "/rock/");
			type->mesh = loader.load();
			type->modifyMaterials([](Material& m) { m.reflection = 0.7f; });
		}

		Node* node = new Node(type);
		Node* planet = getPlanet();
		glm::vec3 pos = planet ? planet->getPos() - glm::vec3(0, 50, 0) : glm::vec3(10, 50, 100) + assets.groundOffset;
		node->setPos(pos);
		scene->registry.addNode(node);
	});
}

void TestSceneSetup::setupNodeAsteroidBelt()
{
	loader.addLoader([this]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
		type->batchMode = false;

		MeshLoader loader(assets, "rock", "/rock/");
		type->mesh = loader.load();

		Node* planet = getPlanet();
		AsteroidBeltController* controller = new AsteroidBeltController(assets, planet);
		InstancedNode* node = new InstancedNode(type, controller);
		//node->selected = true;
		//std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		this->scene->registry.addNode(node);
	});
}

void TestSceneSetup::setupSpriteSkeleton()
{
	loader.addLoader([this]() {
		//auto type = Sprite::getNodeType(assets, "Skeleton_VH.PNG", "Skeleton_VH_normal.PNG");
		auto type = Sprite::getNodeType(assets, scene->shaders, "Skeleton_VH.PNG", "");

		glm::vec3 pos = glm::vec3(0, 5, 20) + assets.groundOffset;
		for (int x = 0; x < 10; x++) {
			for (int z = 0; z < 101; z++) {
				auto sprite = new Sprite(type, glm::vec2(1.5, 3));
				sprite->setPos(pos + glm::vec3(15 - x * 4, 1.5, 0.2 * z));
				//sprite->setRotation(glm::vec3(0, 0, 180));
				scene->registry.addNode(sprite);
			}
		}
	});
}

void TestSceneSetup::setupTerrain()
{
	loader.addLoader([this]() {
		std::shared_ptr<Material> material = std::make_shared<Material>("terrain", assets.texturesDir + "/");
		material->textureSpec.mode = GL_REPEAT;
		material->tiling = 60;
		material->ns = 50;
		material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
		material->map_kd = "Grass Dark_VH.PNG";
		//material->map_kd = "singing_brushes.png";
		material->loadTextures();

		auto shader = loader.getShader(TEX_TERRAIN);

		TerrainGenerator generator(assets);

		for (int x = 0; x < 2; x++) {
			for (int z = 0; z < 2; z++) {
				auto type = std::make_shared<NodeType>(NodeType::nextID(), shader);
				//type->renderBack = true;
				type->noShadow = true;
				type->mesh = generator.generateTerrain(material);

				Terrain* terrain = new Terrain(type, x, 0, z);
				scene->registry.addNode(terrain);
			}
		}
	});
}

void TestSceneSetup::setupWaterBottom()
{
	loader.addLoader([this]() {
		auto type = std::make_shared<NodeType>(NodeType::nextID(), loader.getShader(TEX_TEXTURE));
		//type->renderBack = true;
		type->noShadow = true;
		{
			MeshLoader loader(assets, "marble_plate");
			loader.loadTextures = false;
			type->mesh = loader.load();
			type->modifyMaterials([](Material& m) {
				m.textureSpec.mode = GL_REPEAT;
				m.tiling = 8;
				m.loadTextures();
			});
		}

		glm::vec3 pos = assets.groundOffset;

		Node* node = new Node(type);
		node->setPos(pos + glm::vec3(0, 3, -10));
		node->setScale(30.f);
		node->setRotation({ 90, 0, 0 });
		scene->registry.addNode(node);
	});
}

void TestSceneSetup::setupWaterSurface()
{
	loader.addLoader([this]() {
		std::shared_ptr<Material> material = std::make_shared<Material>("water_surface", assets.modelsDir);
		material->ns = 150;
		material->ks = glm::vec4(0.2f, 0.2f, 0.5f, 1.f);
		material->kd = glm::vec4(0.0f, 0.1f, 0.8f, 1.f);
		//material->map_kd = "CD3B_Water 1_HI.PNG";
		//material->map_bump = "CD3B_Water 1_HI_normal_surface.PNG";
		material->map_bump = "waterNormalMap.png";
		material->map_dudv = "waterDUDV.png";
		material->tiling = 2;
		material->textureSpec.mode = GL_REPEAT;
		//		material->pattern = 1;
		material->loadTextures();
		std::shared_ptr<Shader> shader = loader.getShader(TEX_WATER);

		TerrainGenerator generator(assets);

		auto type = std::make_shared<NodeType>(NodeType::nextID(), shader);
		type->renderBack = true;
		type->water = true;
//		type->blend = true;
		type->noShadow = true;
		type->mesh = generator.generateWater(material);

		glm::vec3 pos = assets.groundOffset;
		Water* water = new Water(type, pos.x, pos.y + 5, pos.z);
		water->setPos(pos + glm::vec3(0, 3.5, -10));
		water->setScale(30);
		water->setRotation({ 270, 0, 0 });

		scene->registry.addNode(water);
	});
}

void TestSceneSetup::setupEffectExplosion()
{
	loader.addLoader([this]() {
		std::shared_ptr<Shader> shader = loader.getShader(TEX_EFFECT);

		TerrainGenerator generator(assets);

		auto type = std::make_shared<NodeType>(NodeType::nextID(), shader);
		type->renderBack = true;
		type->noShadow = true;

		glm::vec3 pos = assets.groundOffset;
		Billboard* node = new Billboard(type);
		node->setPos(pos + glm::vec3(0, 3.5, -20));
		node->setScale(2);

		scene->registry.addNode(node);
	});
}

void TestSceneSetup::setupViewport1()
{
	TextureSpec spec;
	// NOTE KI memory_leak
	PlainTexture* texture = new PlainTexture("checkerboard", spec, 1, 1);
	texture->prepare();

	unsigned int color = 0x90ff2020;
	texture->setData(&color, sizeof(unsigned int));

	auto viewport = std::make_shared<Viewport>(
		glm::vec3(-1, -0.75, 0),
		glm::vec3(0, 0, 0),
		glm::vec2(0.25f, 0.25f),
		texture->textureID,
		scene->shaders.getShader(assets, TEX_VIEWPORT));
	viewport->prepare();
	scene->registry.addViewPort(viewport);
}

void TestSceneSetup::setPlanet(Node* planet)
{
	{
		std::lock_guard<std::mutex> lock(planet_lock);
		loadedPlanet = planet;
		planetFutureIndex = -1;
	}
}

Node* TestSceneSetup::getPlanet()
{
	int index = -1;
	{
		std::lock_guard<std::mutex> lock(planet_lock);
		index = planetFutureIndex;
	}

	if (index == -1) {
		return loadedPlanet;
	}

	loader.getLoader(index).wait();

	{
		std::lock_guard<std::mutex> lock(planet_lock);
		return loadedPlanet;
	}
}
