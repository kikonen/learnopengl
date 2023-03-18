#include "MeshTypeRegistry.h"


MeshTypeRegistry::MeshTypeRegistry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
{
}

MeshTypeRegistry::~MeshTypeRegistry()
{
    for (auto& type : m_types) {
        delete type;
    }
}

MeshType* MeshTypeRegistry::getType(
    const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_lock);

    MeshType * type = new MeshType(name);
    m_types.push_back(type);
    return type;
}

void MeshTypeRegistry::bind(const RenderContext& ctx)
{
    std::lock_guard<std::mutex> lock(m_lock);
    for (auto* type : m_types) {
        type->bind(ctx);
    }
}
