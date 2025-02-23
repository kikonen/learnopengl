#include "MeshTypeRegistry.h"

#include "pool/TypeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "render/RenderContext.h"

#include "material/CustomMaterial.h"

namespace {
    static MeshTypeRegistry g_registry;
}

MeshTypeRegistry& MeshTypeRegistry::get() noexcept
{
    return g_registry;
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
    assert(type->m_customMaterial);

    m_customMaterialTypes.push_back(typeHandle);
}

void MeshTypeRegistry::updateMaterials(const RenderContext& ctx)
{
    for (auto& typeHandle : m_customMaterialTypes) {
        typeHandle.toType()->m_customMaterial.get()->updateRT(ctx);
    }
}

void MeshTypeRegistry::bindMaterials(const RenderContext& ctx)
{
    for (auto& typeHandle : m_customMaterialTypes) {
        typeHandle.toType()->m_customMaterial->bindTextures(ctx.m_state);
    }
}
