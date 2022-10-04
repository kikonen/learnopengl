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
    constexpr auto PLANET_UUID = KI_UUID("8712cec1-e1a3-4973-8889-533adfbbb196");
}

TestSceneSetup::TestSceneSetup(
    AsyncLoader* asyncLoader,
    const Assets& assets)
    : assets(assets), asyncLoader(asyncLoader)
{
}

void TestSceneSetup::setup(std::shared_ptr<Scene> scene)
{
    this->scene = scene;

    if (true) {
        setupNodeActive();

        setupRockLight();
        setupNodeAsteroidBelt();
    }

    if (true) {
        //setupEffectExplosion();

        //setupViewport1();

        setupLightMoving();
    }
}

void TestSceneSetup::setupLightMoving()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        auto type = std::make_shared<NodeType>();
        type->nodeShader = asyncLoader->getShader(TEX_LIGHT);
        type->flags.light = true;
        type->flags.noShadow = true;

        MeshLoader loader(assets, "Moving light", "light");
        type->mesh = loader.load();

        const float radius = 10.f;

        for (int x = 0; x < 2; x++) {
            for (int z = 0; z < 2; z++) {
                auto light = new Light();
                {
                    // 160
                    if (true) {
                        light->point = true;
                        light->linear = 0.14f;
                        light->quadratic = 0.07f;
                    }

                    if (false) {
                        light->spot = true;
                        light->cutoffAngle = 12.5f;
                        light->outerCutoffAngle = 25.f;
                        light->setWorldTarget(glm::vec3(0.0f) + assets.groundOffset);
                    }

                    light->ambient = { 0.4f, 0.4f, 0.2f, 1.f };
                    light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
                    light->specular = { 1.0f, 1.0f, 0.9f, 1.f };
                }

                auto node = new Node(type);
                {
                    node->parentId = KI_UUID("65ce67c8-3efe-4b04-aaf9-fe384152c547");
                    node->setScale(0.5f);

                    node->light.reset(light);

                    {
                        const auto center = glm::vec3(0 + x * radius * 3, 7 + x + z, z * radius * 3);
                        node->controller = std::make_unique<MovingLightController>(center, 10.f, 2.f);
                    }
                }
                scene->registry.addNode(node);
            }
        }
     });
}

void TestSceneSetup::setupNodeBrickwallBox()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        auto type = std::make_shared<NodeType>();
        type->nodeShader = asyncLoader->getShader(TEX_TEXTURE);
        type->flags.renderBack = true;

        MeshLoader loader(assets, "Brickbox", "brickwall2");
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
        auto type = std::make_shared<NodeType>();
        type->nodeShader = asyncLoader->getShader(TEX_TEXTURE);

        MeshLoader loader(assets, "Active cube", "texture_cube");
        type->mesh = loader.load();

        auto node = new Node(type);
        
        node->setPos(glm::vec3{ 0, 0, 0 } + assets.groundOffset);
        node->controller = std::make_unique<NodePathController>(0);

        scene->registry.addNode(node);
     });
}

void TestSceneSetup::setupRockLight()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        auto light = std::make_unique<Light>();
        {
            // 325 = 0.014    0.0007
            light->point = true;
            light->linear = 0.014f;
            light->quadratic = 0.0007f;

            light->ambient = { 0.4f, 0.4f, 0.2f, 1.f };
            light->diffuse = { 0.8f, 0.8f, 0.7f, 1.f };
            light->specular = { 1.0f, 1.0f, 0.9f, 1.f };
        }

        auto type = std::make_shared<NodeType>();
        type->nodeShader = asyncLoader->getShader(TEX_LIGHT);
        {
            type->flags.light = true;
            type->flags.noShadow = true;

            MeshLoader loader(assets, "Planet light", "light");
            loader.forceDefaultMaterial = true;
            type->mesh = loader.load();
        }

        auto node = new Node(type);
        node->parentId = KI_UUID("7c90bc35-1a05-4755-b52a-1f8eea0bacfa");
        node->setScale(0.5f);

        node->light.reset(light.release());

        {
            glm::vec3 center{ 0, 10, 0 };
            node->controller = std::make_unique<MovingLightController>(center, 10.f, 3.f);
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
        auto type = std::make_shared<NodeType>();
        type->nodeShader = asyncLoader->getShader(TEX_TEXTURE);
        type->flags.batchMode = false;

        MeshLoader loader(assets, "Asteroids", "rock", "rock");
        type->mesh = loader.load();

        auto node = new InstancedNode(type);
        node->parentId = PLANET_UUID;
        node->id = KI_UUID("4a935562-0fcb-4dfd-b41e-6c09976f2d2a");

        const int count = assets.asteroidCount;

        node->controller = std::make_unique<AsteroidBeltController>(count);
        scene->registry.addNode(node);
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

        auto type = std::make_shared<NodeType>();
        type->boundShader = shader;
        type->flags.renderBack = true;
        type->flags.noShadow = true;

        auto node = new Billboard(type);
        node->setPos(glm::vec3{ 0, 3.5, -20 } + assets.groundOffset);
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
