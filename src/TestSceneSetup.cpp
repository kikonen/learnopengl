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
    constexpr uuids::uuid PLANET_UUID = uuids::uuid::from_string("8712cec1-e1a3-4973-8889-533adfbbb196").value();
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

    if (true) {
        setupNodeActive();

        setupNodePlanet();
        setupNodeAsteroidBelt();
    }

    setupSpriteSkeleton();

    if (true) {
        //setupTerrain();

        //setupEffectExplosion();

        //setupViewport1();

        setupLightDirectional();
        setupLightMoving();
    }
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
        type->flags.light = true;
        type->flags.noShadow = true;

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

            auto planet = asyncLoader->waitNode(PLANET_UUID, true);
            if (planet) {
                center = planet->getPos();
            }

            node->controller = std::make_unique<MovingLightController>(center, radius, speed, node);
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
        type->flags.light = true;
        type->flags.noShadow = true;
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
                node->controller = std::make_unique<MovingLightController>(center, 10.f, 2.f, node);
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
        type->flags.renderBack = true;

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
        active->controller = std::make_unique<NodePathController>(0);
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
        auto planet = asyncLoader->waitNode(PLANET_UUID, true);

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
            type->flags.light = true;
            type->flags.noShadow = true;

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
            node->controller = std::make_unique<MovingLightController>(center, 4.f, 2.f, node);
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
        type->flags.batchMode = false;

        MeshLoader loader(assets, "rock", "rock");
        type->mesh = loader.load();

        auto planet = asyncLoader->waitNode(PLANET_UUID, true);
        auto node = new InstancedNode(type);
        node->controller = std::make_unique<AsteroidBeltController>(planet);
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
        int countZTop = 0;

        type->batch.batchSize = countX * countZ;

        for (int x = 0; x < countX; x++) {
            // NOTE KI *INTENTIONALLY* a bit more than single buffer can handle
            for (int z = 0; z < countZ + countZTop; z++) {
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
        type->flags.noShadow = true;
        type->mesh = generator.generateTerrain(assets, material);

        for (int x = 0; x < 2; x++) {
            for (int z = 0; z < 2; z++) {
                auto terrain = new Terrain(type, x, 0, z);
                scene->registry.addNode(terrain);
            }
        }
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
        type->flags.renderBack = true;
        type->flags.noShadow = true;

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
