#include "Registry.h"


Registry::Registry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
{
    m_shaderRegistry = std::make_unique<ShaderRegistry>(assets, m_alive);
    m_materialRegistry = std::make_unique<MaterialRegistry>(assets, m_alive);
    m_typeRegistry = std::make_unique<MeshTypeRegistry>(assets, m_alive);
    m_modelRegistry = std::make_unique<ModelRegistry>(assets, m_alive);
    m_nodeRegistry = std::make_unique<NodeRegistry>(assets, m_alive);
    m_entityRegistry = std::make_unique<EntityRegistry>(assets);
}

void Registry::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    m_materialRegistry->prepare();
    m_entityRegistry->prepare();
    m_modelRegistry->prepare();

    m_nodeRegistry->prepare(this);
}