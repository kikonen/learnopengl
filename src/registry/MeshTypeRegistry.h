#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "asset/Assets.h"

#include "pool/TypeHandle.h"

class RenderContext;

//
// NOTE KI main purpsoe of this registry is to delete MeshType instances
//
class MeshTypeRegistry {
public:
    MeshTypeRegistry(
        const Assets& assets);

    ~MeshTypeRegistry();

    void registerCustomMaterial(
        pool::TypeHandle typeHandle);

    void bind(const RenderContext& ctx);

private:
    const Assets& m_assets;

    mutable std::mutex m_lock{};

    std::vector<pool::TypeHandle> m_customMaterialTypes;
};
