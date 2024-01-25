#include "MeshTypeRegistry.h"

#include "pool/TypeHandle.h"

#include "mesh/MeshType.h"

namespace {
}

MeshTypeRegistry::MeshTypeRegistry(
    const Assets& assets)
    : m_assets(assets)
{
}

MeshTypeRegistry::~MeshTypeRegistry()
{
    pool::TypeHandle::clear();
}

void MeshTypeRegistry::registerCustomMaterial(
    pool::TypeHandle typeHandle)
{
    std::lock_guard<std::mutex> lock(m_lock);

    auto* type = typeHandle.toType();
    assert(!type.m_customMaterial);

    m_customMaterialTypes.push_back(typeHandle);
}

void MeshTypeRegistry::bind(const RenderContext& ctx)
{
    std::lock_guard<std::mutex> lock(m_lock);

    for (auto& typeHandle : m_customMaterialTypes) {
        auto* type = typeHandle.toType();
        if (!type->isReady()) continue;
        type->bind(ctx);
    }
}
