#include "RigContainer.h"

#include <assimp/scene.h>

#include <fmt/format.h>

#include "Animation.h"
#include "RigJoint.h"
#include "BoneInfo.h"
#include "VertexBone.h"

namespace animation {
    RigContainer::~RigContainer() = default;

    animation::RigJoint& RigContainer::addJoint(const aiNode* node)
    {
        auto& rigJoint = m_joints.emplace_back(node);
        rigJoint.m_index = static_cast<int16_t>(m_joints.size() - 1);
        return rigJoint;
    }

    animation::BoneInfo& RigContainer::registerBone(const aiBone* bone) noexcept
    {
        auto& bi = m_boneContainer.registerBone(bone);
        auto* rigJoint = findJoint(bi.m_jointName);

        if (bi.m_jointIndex >= 0) {
            assert(rigJoint->m_index == bi.m_jointIndex);
            return bi;
        }

        if (rigJoint) {
            m_boneContainer.bindJoint(bi.m_index, rigJoint->m_index);
        }

        return bi;
    }

    const animation::RigJoint* RigContainer::getJoint(int16_t index) const noexcept
    {
        return &m_joints[index];
    }

    void RigContainer::addAnimation(std::unique_ptr<animation::Animation> animation)
    {
        m_animations.push_back(std::move(animation));
    }

    const animation::RigJoint* RigContainer::findJoint(const std::string& name) const noexcept
    {
        const auto& it = std::find_if(
            m_joints.begin(),
            m_joints.end(),
            [&name](const RigJoint& m) { return m.m_name == name; });
        return it != m_joints.end() ? &m_joints[it->m_index] : nullptr;
    }

    bool RigContainer::hasBones() const noexcept
    {
        return m_boneContainer.hasBones();
    }

    int16_t RigContainer::registerSocket(const std::string& jointName)
    {
        const auto& socketIt = m_NameToSocket.find(jointName);
        if (socketIt != m_NameToSocket.end()) {
            return socketIt->second;
        }

        const auto& jointIt = std::find_if(
            m_joints.begin(),
            m_joints.end(),
            [&jointName](const auto& joint) { return joint.m_name == jointName; });
        if (jointIt == m_joints.end()) return -1;

        auto& joint = *jointIt;

        int16_t index = static_cast<int16_t>(m_sockets.size());

        m_sockets.push_back(joint.m_index);
        m_NameToSocket.insert({ joint.m_name, index });

        return index;
    }

    const animation::RigJoint* RigContainer::getSocket(int16_t socketIndex) const noexcept
    {
        const auto& it = std::find(m_sockets.begin(), m_sockets.end(), socketIndex);
        if (it == m_sockets.end()) nullptr;
        return &m_joints[*it];
    }

    void RigContainer::prepare()
    {
        validate();

        // Mark bones required for rigged animation
        // - all joints with bone
        // - all parent joints of bone
        for (auto& rigJoint : m_joints) {
            auto* bone = m_boneContainer.findByJointIndex(rigJoint.m_index);
            if (!bone) continue;

            rigJoint.m_boneRequired = true;
            rigJoint.m_boneIndex = bone->m_index;

            for (auto jointIndex = rigJoint.m_parentIndex; jointIndex >= 0;) {
                auto& parent = m_joints[jointIndex];
                if (parent.m_boneRequired) break;
                parent.m_boneRequired = true;
                jointIndex = parent.m_parentIndex;
            }
        }

        // NOTE KI mesh required for calculating transforms for attached meshes
        for (auto& rigJoint : m_joints) {
            const auto& it = std::find_if(
                m_sockets.begin(),
                m_sockets.end(),
                [&rigJoint](const uint16_t index) { return index == rigJoint.m_index; });
            if (it == m_sockets.end()) continue;

            rigJoint.m_socketRequired = true;

            for (auto jointIndex = rigJoint.m_parentIndex; jointIndex >= 0;) {
                auto& parent = m_joints[jointIndex];
                // NOTE KI m_sockets is not sorted
                //if (parent.m_socketRequired) break;
                parent.m_socketRequired = true;
                jointIndex = parent.m_parentIndex;
            }
        }
    }

    void RigContainer::validate() const
    {
        // NOTE KI check that all bones are related to some joint
        // - every bone has joint
        // - not every joint has bone
        for (const auto& it : m_boneContainer.m_jointNameToIndex) {
            const auto& name = it.first;

            const auto& jointIt = std::find_if(
                m_joints.begin(),
                m_joints.end(),
                [&name](const auto& rigJoint) {
                    return rigJoint.m_name == name;
                });

            if (jointIt == m_joints.end()) throw std::runtime_error(fmt::format("missing_bone_joint: {}", name));
        }
    }

    //void RigContainer::calculateInvTransforms() noexcept
    //{
    //    for (auto& rigJoint : m_joints) {
    //        rigJoint.m_globalInvTransform = glm::inverse(rigJoint.m_globalTransform);
    //    }
    //}
}
