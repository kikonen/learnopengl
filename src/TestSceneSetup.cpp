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


namespace {
    uuids::uuid planetUUID;

    const std::string PLANET_UUID{ "8712cec1-e1a3-4973-8889-533adfbbb196" };

    const uuids::uuid getPlanetID() {
        if (planetUUID.is_nil()) {
            planetUUID = uuids::uuid::from_string(PLANET_UUID).value();
        }
        return planetUUID;
    }
}

TestSceneSetup::TestSceneSetup(
    AsyncLoader* asyncLoader,
    const Assets& assets)
    : assets(assets), asyncLoader(asyncLoader)
{
}

void TestSceneSetup::setup(std::shared_ptr<Scene> scene)
{
    NodeType::setBaseID(10000);

    this->scene = scene;

    setupCamera();

    if (true) {
        setupNodeActive();

        setupNodePlanet();
        setupNodeAsteroidBelt();
    }

    setupSpriteSkeleton();

    if (true) {
        setupTerrain();

        setupWaterSurface();

        //setupEffectExplosion();

        //setupViewport1();

        setupLightDirectional();
        setupLightMoving();
    }
}

void TestSceneSetup::setupCamera()
{
    glm::vec3 front = glm::vec3(0, 0, -1);
    glm::vec3 up = glm::vec3(0, 1, 0);

    glm::vec3 pos = glm::vec3(-10, 7, -20.f) + assets.groundOffset;
    glm::vec3 rotation = glm::vec3(0, 180, 0.f);

    auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_TEXTURE));
    MeshLoader loader(assets, "player");
    type->mesh = loader.load();

    auto node = new Node(type);
    {
        node->allowNormals = false;

        node->setPos(pos);
        node->setRotation(rotation);
        node->setScale(0.8f);

        node->camera = std::make_unique<Camera>(pos, front, up);
        node->camera->setRotation(rotation);

        node->controller = new CameraController();

        ParticleDefinition pd;
        node->particleGenerator = std::make_unique<ParticleGenerator>(pd);
    }

    scene->registry.addNode(node);
}

void TestSceneSetup::setupLightDirectional()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        // sun
        auto light = std::make_unique<Light>();
        {
            light->setPos(glm::vec3(10, 40, 10) + assets.groundOffset);
            light->setTarget(glm::vec3(0.0f) + assets.groundOffset);

            light->directional = true;

            light->ambient = { 0.4f, 0.4f, 0.4f, 1.f };
            light->diffuse = { 0.4f, 0.4f, 0.4f, 1.f };
            light->specular = { 0.0f, 0.7f, 0.0f, 1.f };
        }

        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_LIGHT));
        type->light = true;
        type->noShadow = true;

        MeshLoader loader(assets, "light");
        loader.defaultMaterial.kd = light->specular;
        loader.overrideMaterials = true;
        type->mesh = loader.load();

        auto node = new Node(type);
        node->setPos(light->getPos());
        node->setScale(1.5f);
        node->light.reset(light.release());

        {
            const float radius = 80.0f;
            const float speed = 20.f;
            glm::vec3 center = glm::vec3(0, 40, 0) + assets.groundOffset;

            auto planet = asyncLoader->waitNode(getPlanetID(), true);
            if (planet) {
                center = planet->getPos();
            }

            node->controller = new MovingLightController(center, radius, speed, node);
        }

        scene->registry.addNode(node);
        });
}

void TestSceneSetup::setupLightMoving()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        //Light* active = nullptr;
        std::vector<std::unique_ptr<Light>> lights;

        float radius = 10.f;

        for (int x = 0; x < 2; x++) {
            for (int z = 0; z < 2; z++) {
                auto light = new Light();
                lights.push_back(std::unique_ptr<Light>(light));

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
            }
        }

        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_LIGHT));
        type->light = true;
        type->noShadow = true;
        //type->wireframe = true;

        MeshLoader loader(assets, "light");
        //loader.overrideMaterials = true;
        type->mesh = loader.load();

        for (auto& light : lights) {
            auto node = new Node(type);
            node->setPos(light->getPos());
            node->setScale(0.5f);
            node->light.reset(light.release());

            {
                glm::vec3 center = node->getPos();
                node->controller = new MovingLightController(center, 10.f, 2.f, node);
            }

            scene->registry.addNode(node);
        }
        });
}

void TestSceneSetup::setupNodeBrickwallBox()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_TEXTURE));
        type->renderBack = true;

        MeshLoader loader(assets, "brickwall2");
        type->mesh = loader.load();

        glm::vec3 pos[] = {
            //        {0.0, 1.0, 0.0},
                    {0.0, -1.0, .0},
                    //        {1.0, 0.0, 0.0},
                    //        {-1.0, 0.0, 0.0},
                    //        {0.0, 0.0, 1.0},
                    //        {0.0, 0.0, -1.0},
        };

        glm::vec3 rot[] = {
            //        {270, 0, 0},
                    {90, 0, 0},
                    //        {0, 90, 0},
                    //        {0, 270, 0},
                    //        {0, 0, 0},
                    //        {0, 180, 0},
        };

        float scale = 100;
        for (int i = 0; i < 1; i++) {
            auto node = new Node(type);
            node->setPos(pos[i] * glm::vec3(scale, scale, scale) + glm::vec3(0, 95, 0) + assets.groundOffset);
            node->setScale(scale);
            node->setRotation(rot[i]);
            //node->skipShadow = true;
            scene->registry.addNode(node);
        }
        });
}

void TestSceneSetup::setupNodeActive()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_TEXTURE));
        MeshLoader loader(assets, "texture_cube");
        type->mesh = loader.load();

        auto active = new Node(type);
        active->controller = new NodePathController(0);
        active->setPos(glm::vec3(0) + assets.groundOffset);
        scene->registry.addNode(active);
        });
}

void TestSceneSetup::setupNodePlanet()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        auto planet = asyncLoader->waitNode(getPlanetID(), true);

        auto light = std::make_unique<Light>();
        {
            glm::vec3 pos = planet ? planet->getPos() - glm::vec3(0, 40, 0) : glm::vec3(0, 40, 0);
            light->setPos(pos);

            // 325 = 0.014    0.0007
            light->point = true;
            light->linear = 0.014f;
            light->quadratic = 0.0007f;

            light->ambient = { 0.4f, 0.4f, 0.2f, 1.f };
            light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
            light->specular = { 1.0f, 1.0f, 0.9f, 1.f };
        }

        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_LIGHT));
        {
            type->light = true;
            type->noShadow = true;

            MeshLoader loader(assets, "light");
            loader.overrideMaterials = true;
            type->mesh = loader.load();
        }

        auto node = new Node(type);
        node->setPos(light->getPos());
        node->setScale(0.5f);

        node->light.reset(light.release());

        {
            glm::vec3 center = node->getPos();
            node->controller = new MovingLightController(center, 4.f, 2.f, node);
        }

        scene->registry.addNode(node);
        });
}

void TestSceneSetup::setupNodeAsteroidBelt()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_TEXTURE));
        type->batchMode = false;

        MeshLoader loader(assets, "rock", "rock");
        type->mesh = loader.load();

        auto planet = asyncLoader->waitNode(getPlanetID(), true);
        auto controller = new AsteroidBeltController(planet);
        auto node = new InstancedNode(type, controller);
        //node->selected = true;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        scene->registry.addNode(node);
        });
}

void TestSceneSetup::setupSpriteSkeleton()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        //auto type = Sprite::getNodeType(assets, "Skeleton_VH.PNG", "Skeleton_VH_normal.PNG");
        auto type = Sprite::getNodeType(assets, asyncLoader->shaders, "Skeleton_VH.PNG", "");

        glm::vec3 pos = glm::vec3(0, 5, 20) + assets.groundOffset;

        int countX = 10;
        int countZ = 100;

        type->batch.batchSize = countX * countZ;

        for (int x = 0; x < countX; x++) {
            // NOTE KI *INTENTIONALLY* a bit more than single buffer can handle
            for (int z = 0; z < countZ + 1; z++) {
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
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        Material material;
        material.name = "terrain";
        material.type = MaterialType::texture;
        material.textureSpec.mode = GL_REPEAT;
        material.tiling = 60;
        material.ns = 50;
        material.ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
        material.map_kd = "Grass Dark_VH.PNG";
        //material.map_kd = "singing_brushes.png";
        material.loadTextures(assets);

        auto shader = asyncLoader->getShader(TEX_TERRAIN);

        TerrainGenerator generator(assets);

        auto type = std::make_shared<NodeType>(NodeType::nextID(), shader);
        //type->renderBack = true;
        type->noShadow = true;
        type->mesh = generator.generateTerrain(assets, material);

        for (int x = 0; x < 2; x++) {
            for (int z = 0; z < 2; z++) {
                auto terrain = new Terrain(type, x, 0, z);
                scene->registry.addNode(terrain);
            }
        }
        });
}

void TestSceneSetup::setupWaterSurface()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        Material material;
        material.name = "water_surface";
        material.ns = 150;
        material.ks = glm::vec4(0.2f, 0.2f, 0.5f, 1.f);
        material.kd = glm::vec4(0.0f, 0.1f, 0.8f, 1.f);
        //material.map_kd = "CD3B_Water 1_HI.PNG";
        //material.map_bump = "CD3B_Water 1_HI_normal_surface.PNG";
        material.map_bump = "waterNormalMap.png";
        material.map_dudv = "waterDUDV.png";
        material.tiling = 2;
        material.textureSpec.mode = GL_REPEAT;
        //        material.pattern = 1;
        material.loadTextures(assets);

        Shader* shader = asyncLoader->getShader(TEX_WATER);

        TerrainGenerator generator(assets);

        auto type = std::make_shared<NodeType>(NodeType::nextID(), shader);
        type->renderBack = true;
        type->water = true;
        //        type->blend = true;
        type->noShadow = true;
        type->mesh = generator.generateWater(assets, material);

        glm::vec3 pos = assets.groundOffset;

        auto water = new Water(type, pos.x, pos.y + 5, pos.z);
        water->setPos(pos + glm::vec3(0, 5, -10));
        water->setScale(30);
        // TODO KI why rotate is 270?!?
        // - due to normals ?!?
        water->setRotation({ 270, 0, 0 });
        //water->setRotation({ 90, 0, 0 });

        scene->registry.addNode(water);
        });
}

void TestSceneSetup::setupEffectExplosion()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        Shader* shader = asyncLoader->getShader(TEX_EFFECT);

        TerrainGenerator generator(assets);

        auto type = std::make_shared<NodeType>(NodeType::nextID(), shader);
        type->renderBack = true;
        type->noShadow = true;

        glm::vec3 pos = assets.groundOffset;

        auto node = new Billboard(type);
        node->setPos(pos + glm::vec3(0, 3.5, -20));
        node->setScale(2);

        scene->registry.addNode(node);
        });
}

void TestSceneSetup::setupViewport1()
{
    TextureSpec spec;
    // NOTE KI memory_leak
    auto texture = new PlainTexture("checkerboard", spec, 1, 1);
    texture->prepare(assets);

    unsigned int color = 0x90ff2020;
    texture->setData(&color, sizeof(unsigned int));

    auto viewport = std::make_shared<Viewport>(
        glm::vec3(-1, -0.75, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.25f, 0.25f),
        texture->textureID,
        asyncLoader->getShader(TEX_VIEWPORT));
    viewport->prepare(assets);
    scene->registry.addViewPort(viewport);
}
