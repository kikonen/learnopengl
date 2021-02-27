#include "SceneLoaderTest.h"

#include "asset/MeshLoader.h"
#include "asset/PlainTexture.h"

#include "model/Sprite.h"
#include "model/Terrain.h"
#include "model/Water.h"
#include "model/InstancedNode.h"

#include "controller/CameraController.h"
#include "controller/AsteroidBeltController.h"
#include "controller/MovingLightController.h"
#include "controller/NodePathController.h"

#include "scene/NodeType.h"
#include "scene/TerrainGenerator.h"


SceneLoaderTest::SceneLoaderTest(const Assets& assets)
	: SceneLoader(assets)
{
}

SceneLoaderTest::~SceneLoaderTest()
{
}

void SceneLoaderTest::setup()
{
	scene = new Scene(assets);

	setupCamera();

	setupNodeZero();

	setupNodeGlassBall();
	setupNodeWaterBall();

	setupNodeCubes();
	setupNodeCube4();

	setupNodeBrickCube();
	setupNodeActive();

	setupNodeBall();
	setupNodeCow();
	setupNodeTeapot();

	setupNodeSpyro();
	setupNodeWindow2();
	setupNodeWindow1();
	setupNodeStainedWindows();

	setupNodeBrickwall();
	setupNodeBigMirror();

	//setupNodeBrickwallBox();
	//setupNodeMountains();

	setupNodePlanet();
	setupNodeAsteroid();
	setupNodeAsteroidBelt();

	setupSpriteSkeleton();

	setupTerrain();

	setupWaterBottom();
	setupWaterSurface();

	setupNodeSkybox();

	//setupNodeBackpack();
	//setupNodeBunny();
	//setupNodeDragon();

	setupViewport1();

	setupLightDirectional();
	setupLightMoving();
}

void SceneLoaderTest::setupCamera()
{
	glm::vec3 pos = glm::vec3(-8, 5, 10.f) + assets.groundOffset;
	glm::vec3 front = glm::vec3(0, 0, -1);
	glm::vec3 up = glm::vec3(0, 1, 0);
	Camera* camera = new Camera(pos, front, up);

	NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
	MeshLoader loader(assets, "player");
	type->mesh = loader.load();

	Node* node = new Node(type);
	node->setPos(pos);
	node->setScale(0.8f);
	node->camera = camera;
	node->controller = new CameraController(assets);

	ParticleDefinition pd;
	node->particleGenerator = new ParticleGenerator(assets, pd);

	scene->registry.addNode(node);
}

void SceneLoaderTest::setupNodeSkybox()
{
	//addLoader([this]() {
	//});
	SkyboxRenderer* skybox = new SkyboxRenderer(assets, "skybox");
	skybox->prepare();

	scene->skyboxRenderer = skybox;
}

void SceneLoaderTest::setupLightDirectional()
{
	// sun
	Light* sun = new Light();
	{
		sun->pos = glm::vec3(10, 40, 10) + assets.groundOffset;
		sun->target = glm::vec3(0.0f) + assets.groundOffset;

		sun->directional = true;

		sun->ambient = { 0.2f, 0.2f, 0.2f, 1.f };
		sun->diffuse = { 0.4f, 0.4f, 0.4f, 1.f };
		sun->specular = { 0.0f, 0.7f, 0.0f, 1.f };
	}

	addLoader([this, sun]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_LIGHT));
		type->light = true;
		type->noShadow = true;

		MeshLoader loader(assets, "light");
		loader.defaultMaterial->kd = sun->specular;
		loader.overrideMaterials = true;
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(sun->pos);
		node->setScale(1.5f);
		node->light = sun;

		scene->registry.addNode(node);

		const float radius = 80.0f;
		const float speed = 20.f;
		glm::vec3 center = glm::vec3(0, 40, 0) + assets.groundOffset;

		Node* planet = getPlanet();
		if (planet) {
			center = planet->getPos();
		}

		node->controller = new MovingLightController(assets, center, radius, speed, node);
	});
}

void SceneLoaderTest::setupLightMoving()
{
	//Light* active = nullptr;
	std::vector<Light*> lights;

	float radius = 10.f;

	for (int x = 0; x < 2; x ++) {
		for (int z = 0; z < 2; z++) {
			Light* light = new Light();

			glm::vec3 center = glm::vec3(0 + x * radius * 3, 7 + x + z, z * radius * 3) + assets.groundOffset;
			light->pos = glm::vec3(10, 5, 10) + assets.groundOffset;
			light->pos = center;

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

			lights.push_back(light);
			//active = light;
		}
	}

	addLoader([this, lights]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_LIGHT));
		type->light = true;
		type->noShadow = true;

		MeshLoader loader(assets, "light");
		loader.overrideMaterials = true;
		type->mesh = loader.load();

		for (auto light : lights) {
			Node* node = new Node(type);
			node->setPos(light->pos);
			node->setScale(0.5f);
			node->light = light;

			{
				//glm::vec3 center = glm::vec3(0, 7, 0) + assets.groundOffset;
				glm::vec3 center = light->pos;
				node->controller = new MovingLightController(assets, center, 10.f, 2.f, node);
			}
		
			scene->registry.addNode(node);
		}
	});
}

void SceneLoaderTest::setupNodeZero() {
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "water_ball");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(0, 0, 0) + assets.groundOffset);
		node->setScale(0.3f);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeWindow1()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type->blend = true;
		type->renderBack = true;

		MeshLoader loader(assets, "window1");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(5, 10, -5) + assets.groundOffset);
		node->setRotation(glm::vec3(0, 180, 0));
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeWindow2()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type->blend = true;
		type->renderBack = true;

		MeshLoader loader(assets, "window2");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(7, 10, -8) + assets.groundOffset);
		node->setRotation(glm::vec3(0, 180, 0));
		scene->registry.addNode(node);
		//	selection.push_back(node);
	});
}

void SceneLoaderTest::setupNodeStainedWindows()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type->blend = true;
		type->renderBack = true;

		MeshLoader loader(assets, "window2");
		type->mesh = loader.load();

		for (int i = 0; i < 10; i++) {
			Node* node = new Node(type);
			node->setPos(glm::vec3(-10 + i * 2, 15, 10) + assets.groundOffset);
			node->setRotation(glm::vec3(0, 180, 0));
			scene->registry.addNode(node);
		}
	});
}

void SceneLoaderTest::setupNodeBrickwall()
{
	addLoader([this]() {
		NodeType* type1 = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type1->renderBack = true;
		{
			MeshLoader loader(assets, "brickwall");
			type1->mesh = loader.load();
		}

		NodeType* type2 = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type2->renderBack = true;
		{
			MeshLoader loader(assets, "woodwall");
			type2->mesh = loader.load();
			type2->setReflection(0.3f);
		}

		for (int i = 0; i < 5; i++) {
			Node* node = new Node(i % 2 == 0 ? type1 : type2);
			node->setPos(glm::vec3(-5 + i * 2, 5, 14) + assets.groundOffset);
			//node->setRotation(glm::vec3(0, 180, 0));
			scene->registry.addNode(node);
		}
	});
}

void SceneLoaderTest::setupNodeBigMirror()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type->renderBack = true;
		
		{
			MeshLoader loader(assets, "woodwall");
			type->mesh = loader.load();
			type->setReflection(0.8f);
		}

		{
			Node* node = new Node(type);
			node->setPos(glm::vec3(-65, 20, -10) + assets.groundOffset);
			node->setRotation(glm::vec3(0, 45, 0));
			node->setScale(15.f);
			scene->registry.addNode(node);
		}

		{
			Node* node = new Node(type);
			node->setPos(glm::vec3(65, 20, -10) + assets.groundOffset);
			node->setRotation(glm::vec3(0, -45, 0));
			node->setScale(15.f);
			scene->registry.addNode(node);
		}
	});
}


void SceneLoaderTest::setupNodeBrickwallBox()
{
	addLoader([this]() {
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
			scene->registry.addNode(node);
		}
	});
}

void SceneLoaderTest::setupNodeSpyro()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "spyro2");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(0, 30, 30) + assets.groundOffset);
		node->setScale(0.1f);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeBackpack()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "backpack", "/backpack/");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(0, 5, 5) + assets.groundOffset);
		node->setScale(1.5f);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeBunny()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "bunny");
		type->renderBack = true;
		type->mesh = loader.load();
		type->setReflection(0.9f);

		Node* node = new Node(type);
		node->setPos(glm::vec3(-15, 10, 5) + assets.groundOffset);
		node->setScale(20.f);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeDragon()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "dragon");
		type->mesh = loader.load();
		type->renderBack = true;
//		type->setReflection(0.6);

		Node* node = new Node(type);
		node->setPos(glm::vec3(-15, 15, 0) + assets.groundOffset);
		node->setScale(30.f);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeTeapot()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type->renderBack = true;

		MeshLoader loader(assets, "smooth_teapot");
		loader.defaultMaterial->kd = glm::vec4(0.578f, 0.578f, 0.168f, 1.f);
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(-5, 20, -5) + assets.groundOffset);
		node->selected = true;
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeCow()
{
	addLoader([this]() {
		//mesh->defaultShader = getShader(TEX_PLAIN, "_explode");
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		{
			MeshLoader loader(assets, "texture_cow");
			loader.defaultMaterial->kd = glm::vec4(0.160f, 0.578f, 0.168f, 1.f);
			type->mesh = loader.load();
			type->setReflection(0.5);
		}

		Node* node = new Node(type);
		node->setPos(glm::vec3(5, 20, -5) + assets.groundOffset);
		node->selected = true;
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeBall()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "texture_ball");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(0, 8, 0) + assets.groundOffset);
		node->setScale(2.0f);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeCube4()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "texture_cube_4");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(-5, 20, 5) + assets.groundOffset);
		node->selected = true;
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeCubes()
{
	addLoader([this]() {
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
			scene->registry.addNode(node);
		}
	});
}

void SceneLoaderTest::setupNodeActive()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "texture_cube");
		type->mesh = loader.load();

		Node* active = new Node(type);
		active->controller = new NodePathController(assets, 0);
		active->setPos(glm::vec3(0) + assets.groundOffset);
		scene->registry.addNode(active);
	});
}

void SceneLoaderTest::setupNodeBrickCube()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "texture_cube");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(5, 20, 5) + assets.groundOffset);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeMountains()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "texture_mountains");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(0));
		//		node->setScale(0.01);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeGlassBall()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));

		{
			MeshLoader loader(assets, "glass_ball");
			type->mesh = loader.load();
			type->setReflection(1.0);
		}

		Node* node = new Node(type);
		node->setPos(glm::vec3(0, 20, 0) + assets.groundOffset);
		node->setScale(1.3f);
		scene->registry.addNode(node);
		});
}

void SceneLoaderTest::setupNodeWaterBall()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));

		MeshLoader loader(assets, "water_ball");
		type->mesh = loader.load();
		type->setReflection(0.3f);

		Node* node = new Node(type);
		node->setPos(glm::vec3(5, 25, 0) + assets.groundOffset);

		ParticleDefinition pd;
		//pd.material = Sprite::getMaterial(assets, "Skeleton_VH.PNG", "");

		node->particleGenerator = new ParticleGenerator(assets, pd);

		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodePlanet()
{
	std::lock_guard<std::mutex> lock(planet_lock);

	planetFutureIndex = addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		MeshLoader loader(assets, "planet", "/planet/");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(10, 100, 100) + assets.groundOffset);
		node->setScale(10);

		scene->registry.addNode(node);
		setPlanet(node);
	});

	addLoader([this]() {
		Node* planet = getPlanet();

		Light* light = new Light();
		{
			light->pos = planet ? planet->getPos() - glm::vec3(0, 40, 0) : glm::vec3(0, 40, 0);

			// 325 = 0.014	0.0007
			light->point = true;
			light->linear = 0.014f;
			light->quadratic = 0.0007f;

			light->ambient = { 0.2f, 0.2f, 0.15f, 1.f };
			light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
			light->specular = { 1.0f, 1.0f, 0.9f, 1.f };
		}

		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_LIGHT));
		{
			type->light = true;
			type->noShadow = true;

			MeshLoader loader(assets, "light");
			loader.overrideMaterials = true;
			type->mesh = loader.load();
		}

		Node* node = new Node(type);
		node->setPos(light->pos);
		node->setScale(0.5f);
		node->light = light;
		scene->registry.addNode(node);

		glm::vec3 center = light->pos;
		node->controller = new MovingLightController(assets, center, 4.f, 2.f, node);

	});
}

void SceneLoaderTest::setupNodeAsteroid()
{
	addLoader([this]() {
		glm::vec3 planetPos = glm::vec3(10, 100, 100);

		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		{
			MeshLoader loader(assets, "rock", "/rock/");
			type->mesh = loader.load();
			type->setReflection(0.7f);
		}

		Node* node = new Node(type);
		Node* planet = getPlanet();
		glm::vec3 pos = planet ? planet->getPos() - glm::vec3(0, 50, 0) : glm::vec3(10, 50, 100) + assets.groundOffset;
		node->setPos(pos);
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupNodeAsteroidBelt()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
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

void SceneLoaderTest::setupSpriteSkeleton()
{
	addLoader([this]() {
		//NodeType* type = Sprite::getNodeType(assets, "Skeleton_VH.PNG", "Skeleton_VH_normal.PNG");
		NodeType* type = Sprite::getNodeType(assets, "Skeleton_VH.PNG", "");

		glm::vec3 pos = glm::vec3(0, 5, 20) + assets.groundOffset;
		for (int x = 0; x < 10; x++) {
			for (int z = 0; z < 101; z++) {
				Sprite* sprite = new Sprite(type, glm::vec2(1.5, 3));
				sprite->setPos(pos + glm::vec3(15 - x * 4, 1.5, 0.2 * z));
				//sprite->setRotation(glm::vec3(0, 0, 180));
				scene->registry.addNode(sprite);
			}
		}
	});
}

void SceneLoaderTest::setupTerrain()
{
	addLoader([this]() {
		Material* material = new Material("terrain");
		material->textureSpec.mode = GL_REPEAT;
		material->ns = 50;
		material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
		material->map_kd = "Grass Dark_VH.PNG";
		//material->map_kd = "singing_brushes.png";	
		material->loadTextures(assets.texturesDir + "/");

		Shader* shader = getShader(TEX_TERRAIN);

		TerrainGenerator generator(assets);

		for (int x = 0; x < 2; x++) {
			for (int z = 0; z < 2; z++) {
				NodeType* type = new NodeType(NodeType::nextID(), shader);
				//type->renderBack = true;
				type->noShadow = true;
				type->mesh = generator.generateTerrain(material);

				Terrain* terrain = new Terrain(type, x, 0, z);
				scene->registry.addNode(terrain);
			}
		}
	});
}

void SceneLoaderTest::setupWaterBottom()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));
		type->renderBack = true;
		{
			MeshLoader loader(assets, "woodwall");
			type->mesh = loader.load();
		}

		glm::vec3 pos = assets.groundOffset;

		Node* node = new Node(type);
		node->setPos(pos + glm::vec3(0, 4, -10));
		node->setScale(10.f);
		node->setRotation({ 90, 0, 0 });
		scene->registry.addNode(node);
	});
}

void SceneLoaderTest::setupWaterSurface()
{
	addLoader([this]() {
		Material* material = new Material("water_surface");
		material->ns = 100;
		material->ks = glm::vec4(0.1f, 0.1f, 0.9f, 1.f);
		material->kd = glm::vec4(0.1f, 0.1f, 0.9f, 1.f);
//		material->pattern = 1;
		Shader* shader = getShader(TEX_WATER);

		TerrainGenerator generator(assets);

		NodeType* type = new NodeType(NodeType::nextID(), shader);
		type->renderBack = true;
		type->water = true;
//		type->blend = true;
		type->noShadow = true;
		type->mesh = generator.generateWater(material);

		glm::vec3 pos = assets.groundOffset;
		Water* water = new Water(type, pos.x, pos.y + 5, pos.z);
		water->setPos(pos + glm::vec3(0, 7, -10));
		water->setScale(10);
		water->setRotation({ 90, 0, 0 });

		scene->registry.addNode(water);
	});
}

void SceneLoaderTest::setupViewport1()
{
	TextureSpec spec;
	PlainTexture* texture = new PlainTexture("checkerboard", spec, 1, 1);
	texture->prepare();

	unsigned int color = 0x90ff2020;
	texture->setData(&color, sizeof(unsigned int));

	Viewport* viewport = new Viewport(
		glm::vec3(-1, -0.75, 0),
		glm::vec3(0, 0, 0),
		glm::vec2(0.25f, 0.25f),
		texture->textureID,
		Shader::getShader(assets, TEX_VIEWPORT));
	viewport->prepare();
	scene->registry.addViewPort(viewport);
}

void SceneLoaderTest::setPlanet(Node* planet)
{
	{
		std::lock_guard<std::mutex> lock(planet_lock);
		loadedPlanet = planet;
		planetFutureIndex = -1;
	}
}

Node* SceneLoaderTest::getPlanet()
{
	int index = -1;
	{
		std::lock_guard<std::mutex> lock(planet_lock);
		index = planetFutureIndex;
	}

	if (index == -1) {
		return loadedPlanet;
	}

	getLoader(index).wait();

	{
		std::lock_guard<std::mutex> lock(planet_lock);
		return loadedPlanet;
	}
}

