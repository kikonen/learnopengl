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
        //setupNodeBrickwallBox();

        //setupEffectExplosion();

        //setupViewport1();
    }
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

        MeshLoader loader(assets, "Huge woodbox", "woodwall");
        type->mesh = loader.load();

        glm::vec3 positions[] = {
            {0.0, 1.0, 0.0},
            {0.0, -1.0, .0},
            {1.0, 0.0, 0.0},
            {-1.0, 0.0, 0.0},
            {0.0, 0.0, 1.0},
            {0.0, 0.0, -1.0},
        };

        glm::vec3 rotations[] = {
            {270, 0, 0},
            {90, 0, 0},
            {0, 90, 0},
            {0, 270, 0},
            {0, 0, 0},
            {0, 180, 0},
        };

        float scale = 100;
        for (int i = 0; i < 6; i++) {
            auto node = new Node(type);
            auto pos = positions[i] * glm::vec3(scale, scale, scale) + glm::vec3(0, 70, 0) + assets.groundOffset;
            node->setPos(pos);
            node->setScale(scale);
            node->setRotation(rotations[i]);
            //node->skipShadow = true;

            scene->registry.addNode(node);
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
