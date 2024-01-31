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
    MeshTypeRegistry();

    ~MeshTypeRegistry();

    void registerCustomMaterial(
        pool::TypeHandle typeHandle);

    void bind(const RenderContext& ctx);

private:
    mutable std::mutex m_lock{};

    std::vector<pool::TypeHandle> m_customMaterialTypes;
};
