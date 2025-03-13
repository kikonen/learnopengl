#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "pool/TypeHandle.h"

class RenderContext;

//
// NOTE KI main purpsoe of this registry is to delete MeshType instances
//
class MeshTypeRegistry {
public:
    static void init() noexcept;
    static void release() noexcept;
    static MeshTypeRegistry& get() noexcept;

    MeshTypeRegistry();
    MeshTypeRegistry& operator=(const MeshTypeRegistry&) = delete;

    ~MeshTypeRegistry();

    void clear();

    void registerCustomMaterial(
        pool::TypeHandle typeHandle);

    void updateMaterials(const RenderContext& ctx);

    void bindMaterials(const RenderContext& ctx);

private:
    mutable std::mutex m_lock{};

    std::vector<pool::TypeHandle> m_customMaterialTypes;
};
