#pragma once

#include <entt/entt.hpp>

#include "asset/Assets.h"

class Registry;

class ComponentRegistry {
    ComponentRegistry(const Assets& assets)
        : m_assets(assets)
    {}

    void prepare(Registry* registry);

private:
    const Assets& m_assets;

    entt::registry m_registry;
};
