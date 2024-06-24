#include "RigContainer.h"

#include <assimp/scene.h>

#include <fmt/format.h>

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

    void RigContainer::addSocket(const RigNode& rigNode)
    {
        const auto& it = std::find_if(
            m_sockets.begin(),
            m_sockets.end(),
            [&rigNode](const uint16_t index) { return index == rigNode.m_index; });
        if (it != m_sockets.end()) return;

        m_sockets.push_back(rigNode.m_index);
        std::sort(m_sockets.begin(), m_sockets.end());
    }

    bool RigContainer::hasSockets() const noexcept
    {
        return !m_sockets.empty();
    }

    void RigContainer::prepare()
    {
        validate();

        // Mark bones required for rigged animation
        // - all nodes with bone
        // - all parent nodes of bone
        for (auto& rigNode : m_nodes) {
            auto* bone = m_boneContainer.findByNodeIndex(rigNode.m_index);
            if (!bone) continue;

            rigNode.m_boneRequired = true;
            rigNode.m_boneIndex = bone->m_index;

            for (auto nodeIndex = rigNode.m_parentIndex; nodeIndex >= 0;) {
                auto& parent = m_nodes[nodeIndex];
                if (parent.m_boneRequired) break;
                parent.m_boneRequired = true;
                nodeIndex = parent.m_parentIndex;
            }
        }

        // NOTE KI mesh required for calculating transforms for attached meshes
        for (auto& rigNode : m_nodes) {
            const auto& it = std::find_if(
                m_sockets.begin(),
                m_sockets.end(),
                [&rigNode](const uint16_t index) { return index == rigNode.m_index; });
            if (it == m_sockets.end()) continue;

            rigNode.m_socketRequired = true;

            for (auto nodeIndex = rigNode.m_parentIndex; nodeIndex >= 0;) {
                auto& parent = m_nodes[nodeIndex];
                if (parent.m_socketRequired) break;
                parent.m_socketRequired = true;
                nodeIndex = parent.m_parentIndex;
            }
        }
    }

    void RigContainer::validate() const
    {
        // NOTE KI check that all bones are related to some Node
        // - every bone has node
        // - not every node has bone
        for (const auto& it : m_boneContainer.m_nodeNameToIndex) {
            const auto& name = it.first;

            const auto& nodeIt = std::find_if(
                m_nodes.begin(),
                m_nodes.end(),
                [&name](const auto& node) {
                    return node.m_name == name;
                });

            if (nodeIt == m_nodes.end()) throw fmt::format("missing: {}", name);
        }
    }

    //void RigContainer::calculateInvTransforms() noexcept
    //{
    //    for (auto& rigNode : m_nodes) {
    //        rigNode.m_globalInvTransform = glm::inverse(rigNode.m_globalTransform);
    //    }
    //}
}
