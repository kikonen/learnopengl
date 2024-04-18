#include "AnimationContainer.h"

#include <assimp/scene.h>

#include "AnimationNode.h"

namespace animation {
    AnimationContainer::~AnimationContainer()
    {}

    AnimationNode& AnimationContainer::addNode(const aiNode* node)
    {
        auto& animNode = m_nodes.emplace_back(node);
        animNode.m_id = static_cast<int16_t>(m_nodes.size());
        return animNode;
    }
}
