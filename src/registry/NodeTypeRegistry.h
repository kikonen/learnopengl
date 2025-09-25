#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "pool/TypeHandle.h"

namespace render
{
    class RenderContext;
}

//
// NOTE KI main purpsoe of this registry is to delete NodeType instances
//
class NodeTypeRegistry {
public:
    static void init() noexcept;
    static void release() noexcept;
    static NodeTypeRegistry& get() noexcept;

    NodeTypeRegistry();
    NodeTypeRegistry& operator=(const NodeTypeRegistry&) = delete;

    ~NodeTypeRegistry();

    void clear();

    void registerType(
        pool::TypeHandle typeHandle);

    std::vector<pool::TypeHandle> getTypeHandles() const;

    void registerCustomMaterial(
        pool::TypeHandle typeHandle);

    void updateMaterials(const render::RenderContext& ctx);

    void bindMaterials(const render::RenderContext& ctx);

private:
    mutable std::mutex m_lock{};

    std::vector<pool::TypeHandle> m_types;

    std::vector<pool::TypeHandle> m_customMaterialTypes;
};
