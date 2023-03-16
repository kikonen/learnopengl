#pragma once

#include <atomic>

#include "asset/Assets.h"

#include "event/Dispatcher.h"

#include "registry/ProgramRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"

#include "physics/PhysicsEngine.h"

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
    std::unique_ptr<ProgramRegistry> m_programRegistry;

    std::unique_ptr<MaterialRegistry> m_materialRegistry;
    std::unique_ptr<MeshTypeRegistry> m_typeRegistry;
    std::unique_ptr<ModelRegistry> m_modelRegistry;
    std::unique_ptr<NodeRegistry> m_nodeRegistry;
    std::unique_ptr<EntityRegistry> m_entityRegistry;

    std::unique_ptr<physics::PhysicsEngine> m_physicsEngine;

    std::unique_ptr<event::Dispatcher> m_dispatcher;

private:
    const Assets& m_assets;

    bool m_prepared = false;

    std::shared_ptr<std::atomic<bool>> m_alive;
};
