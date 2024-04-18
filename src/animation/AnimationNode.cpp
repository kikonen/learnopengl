#include "AnimationNode.h"

#include <assimp/scene.h>

namespace animation {
    AnimationNode::AnimationNode(const aiNode* node)
        : m_node{ node }
    {
    }
}
