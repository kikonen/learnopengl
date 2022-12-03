#include "TestSceneSetup.h"

#include "asset/MeshLoader.h"
#include "asset/PlainTexture.h"

#include "model/InstancedNode.h"

#include "controller/CameraController.h"
#include "controller/AsteroidBeltController.h"
#include "controller/MovingLightController.h"
#include "controller/NodePathController.h"

#include "registry/MeshType.h"


namespace {
    //constexpr auto PLANET_UUID = KI_UUID("8712cec1-e1a3-4973-8889-533adfbbb196");
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
        //setupEffectExplosion();

        //setupViewport1();
    }
}

void TestSceneSetup::setupEffectExplosion()
{
    auto scene = this->scene;
    auto asyncLoader = this->asyncLoader;
    auto assets = this->assets;

    asyncLoader->addLoader([assets, scene, asyncLoader]() {
        Shader* shader = asyncLoader->getShader(TEX_EFFECT);

        auto type = scene->m_typeRegistry.getType("<effect>");
        type->m_nodeShader = shader;
        type->m_flags.renderBack = true;
        type->m_flags.noShadow = true;

        auto node = new Node(type);
        node->setScale(2);

        scene->m_registry.addNode(type, node);
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
        "Viewport-1",
        glm::vec3(-1, -0.75, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.25f, 0.25f),
        texture->m_textureID,
        asyncLoader->getShader(TEX_VIEWPORT));
    viewport->prepare(assets);
    scene->m_registry.addViewPort(viewport);
}
