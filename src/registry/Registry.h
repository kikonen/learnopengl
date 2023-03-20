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
#include "registry/ViewportRegistry.h"

#include "physics/PhysicsEngine.h"


class UpdateContext;

//
// Container for all registries to simplify passing them around
//
class Registry {
public:
    Registry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    void prepare();

    void update(const UpdateContext& ctx);

public:
    ProgramRegistry* const m_programRegistry;

    MaterialRegistry* const m_materialRegistry;
    MeshTypeRegistry* const m_typeRegistry;
    ModelRegistry* const m_modelRegistry;
    NodeRegistry* const m_nodeRegistry;
    EntityRegistry* const m_entityRegistry;
    ViewportRegistry* const m_viewportRegistry;

    physics::PhysicsEngine* const m_physicsEngine;

    event::Dispatcher* const m_dispatcher;

private:
    const Assets& m_assets;

    bool m_prepared = false;

    std::shared_ptr<std::atomic<bool>> m_alive;

    ProgramRegistry m_programRegistryImpl;

    MaterialRegistry m_materialRegistryImpl;
    MeshTypeRegistry m_typeRegistryImpl;
    ModelRegistry m_modelRegistryImpl;
    NodeRegistry m_nodeRegistryImpl;
    EntityRegistry m_entityRegistryImpl;
    ViewportRegistry m_viewportRegistryImpl;

    physics::PhysicsEngine m_physicsEngineImpl;

    event::Dispatcher m_dispatcherImpl;
};
