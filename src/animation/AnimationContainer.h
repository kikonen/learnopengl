#pragma once

#include <vector>
#include <map>

#include "ki/size.h"

struct aiNode;

namespace animation {
    struct AnimationNode;

    struct AnimationContainer {
        AnimationNode& addNode(const aiNode* node);

        ~AnimationContainer();

        std::vector<AnimationNode> m_nodes;

        std::map<size_t, ki::material_id> m_materialMapping;
    };
}
