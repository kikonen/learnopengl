#include "Registry.h"


Registry::Registry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
{
    m_programRegistry = std::make_unique<ProgramRegistry>(assets, m_alive);
    m_materialRegistry = std::make_unique<MaterialRegistry>(assets, m_alive);
    m_typeRegistry = std::make_unique<MeshTypeRegistry>(assets, m_alive);
    m_modelRegistry = std::make_unique<ModelRegistry>(assets, m_alive);
    m_nodeRegistry = std::make_unique<NodeRegistry>(assets, m_alive);
    m_entityRegistry = std::make_unique<EntityRegistry>(assets);
    m_viewportRegistry = std::make_unique<ViewportRegistry>(assets);

    m_physicsEngine = std::make_unique<physics::PhysicsEngine>(assets);
    m_dispatcher = std::make_unique<event::Dispatcher>(assets);
}

void Registry::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    m_dispatcher->prepare();

    m_materialRegistry->prepare();
    m_entityRegistry->prepare();
    m_modelRegistry->prepare();

    m_viewportRegistry->prepare();

    m_nodeRegistry->prepare(this);

    m_physicsEngine->prepare();
}

void Registry::update(const UpdateContext& ctx)
{
    m_materialRegistry->update(ctx);
    m_modelRegistry->update(ctx);
    m_entityRegistry->update(ctx);
}
