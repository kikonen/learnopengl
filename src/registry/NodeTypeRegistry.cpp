#include "NodeTypeRegistry.h"

#include "util/thread.h"

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

void NodeTypeRegistry::registerType(
    pool::TypeHandle typeHandle)
{
    std::lock_guard lock{ m_lock };

    m_types.push_back(typeHandle);
}

std::vector<pool::TypeHandle> NodeTypeRegistry::getTypeHandles() const
{
    std::vector<pool::TypeHandle> types;
    {
        std::lock_guard lock{ m_lock };
        types = m_types;
    }
    return types;
}

void NodeTypeRegistry::registerCustomMaterial(
    pool::TypeHandle typeHandle)
{
    ASSERT_RT();

    auto* type = typeHandle.toType();
    assert(type->m_customMaterial);

    m_customMaterialTypes.push_back(typeHandle);
}

void NodeTypeRegistry::updateMaterials(const render::RenderContext& ctx)
{
    for (auto& typeHandle : m_customMaterialTypes) {
        typeHandle.toType()->m_customMaterial.get()->updateRT(ctx);
    }
}

void NodeTypeRegistry::bindMaterials(const render::RenderContext& ctx)
{
    for (auto& typeHandle : m_customMaterialTypes) {
        typeHandle.toType()->m_customMaterial->bindTextures(ctx.m_state);
    }
}
