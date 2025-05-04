#include "MeshTypeRegistry.h"

#include "pool/TypeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "render/RenderContext.h"

#include "material/CustomMaterial.h"

namespace
{
    thread_local std::exception_ptr lastException = nullptr;

    static MeshTypeRegistry* s_registry{ nullptr };
}

void MeshTypeRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new MeshTypeRegistry();
}

void MeshTypeRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

MeshTypeRegistry& MeshTypeRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

MeshTypeRegistry::MeshTypeRegistry()
{
    clear();
}

MeshTypeRegistry::~MeshTypeRegistry()
{
    clear();
}

void MeshTypeRegistry::clear()
{
    pool::TypeHandle::clear();
    m_customMaterialTypes.clear();
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
