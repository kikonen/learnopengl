#include "TestSceneSetup.h"

#include "asset/Assets.h"
#include "asset/PlainTexture.h"
#include "asset/Program.h"
#include "asset/Shader.h"

#include "pool/NodeHandle.h"
#include "ki/sid.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/ModelLoader.h"

#include "model/Node.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ProgramRegistry.h"

#include "engine/AsyncLoader.h"

namespace {
}

TestSceneSetup::TestSceneSetup(
    std::shared_ptr<std::atomic<bool>> alive,
    std::shared_ptr<AsyncLoader> asyncLoader)
    : m_alive(alive),
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
        Program* program = ProgramRegistry::get().getProgram(SHADER_EFFECT);

        const auto& assets = Assets::get();

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<effect>");

        type->m_program = program;
        type->m_flags.renderBack = true;
        type->m_flags.noShadow = true;

        auto nodeId = SID("<effect>");
        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
#ifdef _DEBUG
        node->m_resolvedSID = "<effect>";
#endif
        node->m_typeHandle = typeHandle;

        auto& transform = node->modifyTransform();

        transform.setScale(2);

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = node->getId(),
                .parentId = assets.rootId,
            };
            m_registry->m_dispatcherWorker->send(evt);
        }
    });
}

void TestSceneSetup::setupViewport1()
{
    const auto& assets = Assets::get();

    TextureSpec spec;
    // NOTE KI memory_leak
    auto texture = new PlainTexture("checkerboard", false, spec, 1, 1);
    texture->prepare();

    unsigned int color = 0x90ff2020;
    texture->setData(&color, sizeof(unsigned int));

    auto viewport = std::make_shared<Viewport>(
        "Viewport-1",
        glm::vec3(-1, -0.75, 0),
        glm::vec3(0, 0, 0),
        glm::vec2(0.25f, 0.25f),
        false,
        texture->m_textureID,
        ProgramRegistry::get().getProgram(SHADER_VIEWPORT));
    viewport->prepareRT();
    ViewportRegistry::get().addViewport(viewport);
}
