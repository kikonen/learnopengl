#pragma once

#include <atomic>

#include "asset/Assets.h"

#include "registry/ShaderRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"

//
// Container for all registries to simplify passing them around
//
class Registry {
public:
    Registry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    void prepare();

public:
    std::shared_ptr<MaterialRegistry> m_materialRegistry;
    std::shared_ptr<MeshTypeRegistry> m_typeRegistry;
    std::shared_ptr<ModelRegistry> m_modelRegistry;
    std::shared_ptr<NodeRegistry> m_nodeRegistry;

    std::unique_ptr<EntityRegistry> m_entityRegistry;

    std::unique_ptr<ShaderRegistry> m_shaderRegistry;

private:
    const Assets& m_assets;

    bool m_prepared = false;

    std::shared_ptr<std::atomic<bool>> m_alive;
};
