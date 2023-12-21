#include "MeshTypeRegistry.h"

namespace {
    constexpr size_t MAX_TYPES = 10000;
}

MeshTypeRegistry::MeshTypeRegistry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
{
    m_types.reserve(MAX_TYPES);
    // NOTE KI null entries to avoid need for "- 1" math
    m_types.emplace_back<MeshType>({""});
}

MeshTypeRegistry::~MeshTypeRegistry()
{
    m_types.clear();
}

const MeshType* MeshTypeRegistry::getType(ki::type_id id)
{
    std::lock_guard<std::mutex> lock(m_lock);
    return &m_types[id];
}

MeshType* MeshTypeRegistry::modifyType(ki::type_id id)
{
    std::lock_guard<std::mutex> lock(m_lock);
    return &m_types[id];
}

MeshType* MeshTypeRegistry::registerType(
    const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_lock);

    auto& type = m_types.emplace_back<MeshType>({ std::string_view{name} });
    type.m_id = static_cast<ki::type_id>(m_types.size() - 1);

    return &type;
}

void MeshTypeRegistry::bind(const RenderContext& ctx)
{
    std::lock_guard<std::mutex> lock(m_lock);
    for (auto& type : m_types) {
        type.bind(ctx);
    }
}
