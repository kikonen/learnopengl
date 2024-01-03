#pragma once

#include <vector>
#include <map>
#include <mutex>

#include "asset/Assets.h"

#include "mesh/MeshType.h"

class RenderContext;

//
// NOTE KI main purpsoe of this registry is to delete MeshType instances
//
class MeshTypeRegistry {
public:
    MeshTypeRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~MeshTypeRegistry();

    const MeshType* getType(ki::type_id);
    MeshType* modifyType(ki::type_id);

    MeshType* registerType(
        const std::string& name);

    void registerCustomMaterial(
        ki::type_id typeId);

    void bind(const RenderContext& ctx);

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::mutex m_lock{};

    std::vector<MeshType> m_types;

    std::vector<ki::type_id> m_customMaterialTypes;
};
