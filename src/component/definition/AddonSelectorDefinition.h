#pragma once

#include <glm/glm.hpp>

#include "ki/sid.h"

struct Addon
{
    bool enabled{ true };

    ki::sid_t id;
    ki::sid_t group;

    uint32_t seed{ 0 };
    glm::uvec2 range{ 1, 1 };
};

struct AddonSelectorDefinition
{
    std::vector<Addon> addons;
};
