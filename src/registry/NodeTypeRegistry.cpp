#include "NodeTypeRegistry.h"

#include "pool/TypeHandle.h"

#include "mesh/LodMesh.h"

#include "render/RenderContext.h"

#include "material/CustomMaterial.h"

#include "model/NodeType.h"

namespace
{
    thread_local std::exception_ptr lastException = nullptr;

    static NodeTypeRegistry* s_registry{ nullptr };
}

void NodeTypeRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new NodeTypeRegistry();
}

void NodeTypeRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

NodeTypeRegistry& NodeTypeRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

NodeTypeRegistry::NodeTypeRegistry()
{
    clear();
}

NodeTypeRegistry::~NodeTypeRegistry()
{
    clear();
}

void NodeTypeRegistry::clear()
{
    pool::TypeHandle::clear();
    m_customMaterialTypes.clear();
}

void NodeTypeRegistry::registerCustomMaterial(
    pool::TypeHandle typeHandle)
{
    auto* type = typeHandle.toType();
    assert(type->m_customMaterial);

    m_customMaterialTypes.push_back(typeHandle);
}

void NodeTypeRegistry::updateMaterials(const RenderContext& ctx)
{
    for (auto& typeHandle : m_customMaterialTypes) {
        typeHandle.toType()->m_customMaterial.get()->updateRT(ctx);
    }
}

void NodeTypeRegistry::bindMaterials(const RenderContext& ctx)
{
    for (auto& typeHandle : m_customMaterialTypes) {
        typeHandle.toType()->m_customMaterial->bindTextures(ctx.m_state);
    }
}
