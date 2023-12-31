#include "TestSceneSetup.h"

#include "asset/MeshLoader.h"
#include "asset/PlainTexture.h"
#include "asset/Program.h"
#include "asset/Shader.h"

#include "event/Dispatcher.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ProgramRegistry.h"

#include "engine/AsyncLoader.h"

namespace {
    //constexpr auto PLANET_UUID = KI_UUID("8712cec1-e1a3-4973-8889-533adfbbb196");
}

TestSceneSetup::TestSceneSetup(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive,
    std::shared_ptr<AsyncLoader> asyncLoader)
    : m_assets(assets),
    m_alive(alive),
    m_asyncLoader(asyncLoader)
{
}

void TestSceneSetup::setup(
    std::shared_ptr<Registry> registry)
{
    m_registry = registry;

    if (true) {
        //setupEffectExplosion();

        //setupViewport1();
    }
}

void TestSceneSetup::setupEffectExplosion()
{
    m_asyncLoader->addLoader(m_alive, [this]() {
        Program* program = m_registry->m_programRegistry->getProgram(SHADER_EFFECT);

        auto type = m_registry->m_typeRegistry->registerType("<effect>");
        type->m_program = program;
        type->m_flags.renderBack = true;
        type->m_flags.noShadow = true;

        auto node = new Node(type);
        auto& transform = node->modifyTransform();

        transform.setScale(2);

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = node,
                .uuid = {},
                .parentUUID = m_assets.rootUUID,
            };
            m_registry->m_dispatcher->send(evt);
        }
    });
}

void TestSceneSetup::setupViewport1()
{
    TextureSpec spec;
    // NOTE KI memory_leak
    auto texture = new PlainTexture("checkerboard", false, spec, 1, 1);
    texture->prepare(m_assets);

    unsigned int color = 0x90ff2020;
    texture->setData(&color, sizeof(unsigned int));

    auto viewport = std::make_shared<Viewport>(
        "Viewport-1",
        glm::vec3(-1, -0.75, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.25f, 0.25f),
        false,
        texture->m_textureID,
        m_registry->m_programRegistry->getProgram(SHADER_VIEWPORT));
    viewport->prepareRT(m_assets);
    m_registry->m_viewportRegistry->addViewport(viewport);
}
