#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "ki/sid.h"

namespace model
{
    class NodeType;
    class Node;
}

struct AddonDefinition
{
    bool enabled{ true };

    ki::sid_t id;
    ki::sid_t group;

    // 0 == no randomization
    uint32_t seed{ 0 };

    // how many elements to select from group
    // min == [0, n]
    // max = [n, -1], -1 == group size
    glm::uvec2 range{ 1, 1 };
};

struct AddonSelectorDefinition
{
    std::vector<AddonDefinition> addons;

    void selectAddons(
        const model::NodeType* type,
        model::Node* node);
};
