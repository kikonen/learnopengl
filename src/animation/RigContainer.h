#pragma once

#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

struct aiNode;

namespace animation {
    struct RigNode;
    struct Animation;

    struct RigContainer {
        ~RigContainer();

        animation::RigNode& addNode(const aiNode* node);
        void addAnimation(std::unique_ptr<animation::Animation> animation);

        std::vector<animation::RigNode> m_nodes;

        std::map<size_t, ki::material_id> m_materialMapping;

        std::vector<std::unique_ptr<animation::Animation>> m_animations;
    };
}
