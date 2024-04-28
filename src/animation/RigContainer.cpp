#include "RigContainer.h"

#include <assimp/scene.h>

#include "RigNode.h"
#include "Animation.h"

namespace animation {
    RigContainer::~RigContainer() = default;

    animation::RigNode& RigContainer::addNode(const aiNode* node)
    {
        auto& rigNode = m_nodes.emplace_back(node);
        rigNode.m_id = static_cast<int16_t>(m_nodes.size() - 1);
        return rigNode;
    }

    void RigContainer::addAnimation(std::unique_ptr<animation::Animation> animation)
    {
        m_animations.push_back(std::move(animation));
    }

    int16_t RigContainer::findNodeId(const std::string& name) const noexcept
    {
        const auto& it = std::find_if(
            m_nodes.begin(),
            m_nodes.end(),
            [&name](const RigNode& m) { return m.m_name == name; });
        return it != m_nodes.end() ? it->m_id : -1;
    }
}
