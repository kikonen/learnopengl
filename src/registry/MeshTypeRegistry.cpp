#include "MeshTypeRegistry.h"


MeshTypeRegistry::MeshTypeRegistry(const Assets& assets)
    : assets(assets)
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

    MeshType* type = new MeshType(name);
    m_types.push_back(type);
    return type;
}
