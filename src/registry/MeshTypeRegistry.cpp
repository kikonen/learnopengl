#include "MeshTypeRegistry.h"

#include "pool/TypeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

namespace {
}

MeshTypeRegistry& MeshTypeRegistry::get() noexcept
{
    static MeshTypeRegistry s_registry;
    return s_registry;
}

MeshTypeRegistry::MeshTypeRegistry()
{
}

MeshTypeRegistry::~MeshTypeRegistry()
{
    pool::TypeHandle::clear();
}

void MeshTypeRegistry::registerCustomMaterial(
    pool::TypeHandle typeHandle)
{
    auto* type = typeHandle.toType();
    assert(!type->m_customMaterial);

    std::lock_guard lock(m_lock);
    m_customMaterialTypes.push_back(typeHandle);
}

void MeshTypeRegistry::bind(const RenderContext& ctx)
{
    std::lock_guard lock(m_lock);

    for (auto& typeHandle : m_customMaterialTypes) {
        auto* type = typeHandle.toType();
        if (!type->isReady()) continue;
        type->bind(ctx);
    }
}
