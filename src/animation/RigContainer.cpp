#include "RigContainer.h"

#include <assimp/scene.h>

#include "Animation.h"
#include "RigNode.h"
#include "BoneInfo.h"
#include "VertexBone.h"

namespace animation {
    RigContainer::~RigContainer() = default;

    animation::RigNode& RigContainer::addNode(const aiNode* node)
    {
        auto& rigNode = m_nodes.emplace_back(node);
        rigNode.m_index = static_cast<int16_t>(m_nodes.size() - 1);
        return rigNode;
    }

    const animation::RigNode* RigContainer::getNode(int16_t index) const noexcept
    {
        return &m_nodes[index];
    }

    void RigContainer::addAnimation(std::unique_ptr<animation::Animation> animation)
    {
        m_animations.push_back(std::move(animation));
    }

    const animation::RigNode* RigContainer::findNode(const std::string& name) const noexcept
    {
        const auto& it = std::find_if(
            m_nodes.begin(),
            m_nodes.end(),
            [&name](const RigNode& m) { return m.m_name == name; });
        return it != m_nodes.end() ? &m_nodes[it->m_index] : nullptr;
    }

    bool RigContainer::hasBones() const noexcept
    {
        return m_boneContainer.hasBones();
    }
}
