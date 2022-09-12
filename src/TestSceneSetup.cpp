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
    std::shared_ptr<AsyncLoader> asyncLoader,
    const Assets& assets)
    : assets(assets), asyncLoader(asyncLoader)
{
}

TestSceneSetup::~TestSceneSetup()
{
}

void TestSceneSetup::setup(std::shared_ptr<Scene> scene)
{
    NodeType::setBaseID(10000);

    this->scene = scene;

    setupCamera();

    setupNodeActive();

    setupNodePlanet();
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

    auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_TEXTURE));
    MeshLoader loader(assets, "player");
    type->mesh = loader.load();

    auto node = new Node(type);
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

void TestSceneSetup::setupLightDirectional()
{
    asyncLoader->addLoader([this]() {
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
        loader.defaultMaterial->kd = light->specular;
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

            auto planet = asyncLoader->waitNode(getPlanetID());
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
    asyncLoader->addLoader([this]() {
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
                node->controller = new MovingLightController(assets, center, 10.f, 2.f, node);
            }

            scene->registry.addNode(node);
        }
        });
}

void TestSceneSetup::setupNodeBrickwallBox()
{
    asyncLoader->addLoader([this]() {
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
    asyncLoader->addLoader([this]() {
        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_TEXTURE));
        MeshLoader loader(assets, "texture_cube");
        type->mesh = loader.load();

        auto active = new Node(type);
        active->controller = new NodePathController(assets, 0);
        active->setPos(glm::vec3(0) + assets.groundOffset);
        scene->registry.addNode(active);
        });
}

void TestSceneSetup::setupNodePlanet()
{
    asyncLoader->addLoader([this]() {
        auto planet = asyncLoader->waitNode(getPlanetID());

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
            node->controller = new MovingLightController(assets, center, 4.f, 2.f, node);
        }

        scene->registry.addNode(node);
        });
}

void TestSceneSetup::setupNodeAsteroidBelt()
{
    asyncLoader->addLoader([this]() {
        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_TEXTURE));
        type->batchMode = false;

        MeshLoader loader(assets, "rock", "/rock/");
        type->mesh = loader.load();

        auto planet = asyncLoader->waitNode(getPlanetID());
        auto controller = new AsteroidBeltController(assets, planet);
        auto node = new InstancedNode(type, controller);
        //node->selected = true;
        //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        this->scene->registry.addNode(node);
        });
}

void TestSceneSetup::setupSpriteSkeleton()
{
    asyncLoader->addLoader([this]() {
        //auto type = Sprite::getNodeType(assets, "Skeleton_VH.PNG", "Skeleton_VH_normal.PNG");
        auto type = Sprite::getNodeType(assets, asyncLoader->shaders, "Skeleton_VH.PNG", "");

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
    asyncLoader->addLoader([this]() {
        std::shared_ptr<Material> material = std::make_shared<Material>("terrain", assets.texturesDir + "/");
        material->textureSpec.mode = GL_REPEAT;
        material->tiling = 60;
        material->ns = 50;
        material->ks = glm::vec4(0.6f, 0.6f, 0.6f, 1.f);
        material->map_kd = "Grass Dark_VH.PNG";
        //material->map_kd = "singing_brushes.png";
        material->loadTextures(assets);

        auto shader = asyncLoader->getShader(TEX_TERRAIN);

        TerrainGenerator generator(assets);

        for (int x = 0; x < 2; x++) {
            for (int z = 0; z < 2; z++) {
                auto type = std::make_shared<NodeType>(NodeType::nextID(), shader);
                //type->renderBack = true;
                type->noShadow = true;
                type->mesh = generator.generateTerrain(material);

                auto terrain = new Terrain(type, x, 0, z);
                scene->registry.addNode(terrain);
            }
        }
        });
}

void TestSceneSetup::setupWaterBottom()
{
    asyncLoader->addLoader([this]() {
        auto type = std::make_shared<NodeType>(NodeType::nextID(), asyncLoader->getShader(TEX_TEXTURE));
        //type->renderBack = true;
        type->noShadow = true;
        {
            MeshLoader loader(assets, "marble_plate");
            loader.loadTextures = false;
            type->mesh = loader.load();
            type->modifyMaterials([this](Material& m) {
                m.textureSpec.mode = GL_REPEAT;
                m.tiling = 8;
                m.loadTextures(assets);
                });
        }

        glm::vec3 pos = assets.groundOffset;

        auto node = new Node(type);
        node->setPos(pos + glm::vec3(0, 3, -10));
        node->setScale(30.f);
        node->setRotation({ 90, 0, 0 });
        scene->registry.addNode(node);
        });
}

void TestSceneSetup::setupWaterSurface()
{
    asyncLoader->addLoader([this]() {
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
        //        material->pattern = 1;
        material->loadTextures(assets);
        std::shared_ptr<Shader> shader = asyncLoader->getShader(TEX_WATER);

        TerrainGenerator generator(assets);

        auto type = std::make_shared<NodeType>(NodeType::nextID(), shader);
        type->renderBack = true;
        type->water = true;
        //        type->blend = true;
        type->noShadow = true;
        type->mesh = generator.generateWater(material);

        glm::vec3 pos = assets.groundOffset;

        auto water = new Water(type, pos.x, pos.y + 5, pos.z);
        water->setPos(pos + glm::vec3(0, 3.5, -10));
        water->setScale(30);
        water->setRotation({ 270, 0, 0 });

        scene->registry.addNode(water);
        });
}

void TestSceneSetup::setupEffectExplosion()
{
    asyncLoader->addLoader([this]() {
        std::shared_ptr<Shader> shader = asyncLoader->getShader(TEX_EFFECT);

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
    texture->prepare();

    unsigned int color = 0x90ff2020;
    texture->setData(&color, sizeof(unsigned int));

    auto viewport = std::make_shared<Viewport>(
        glm::vec3(-1, -0.75, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.25f, 0.25f),
        texture->textureID,
        asyncLoader->shaders.getShader(assets, TEX_VIEWPORT));
    viewport->prepare();
    scene->registry.addViewPort(viewport);
}
