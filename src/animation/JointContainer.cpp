#include "JointContainer.h"

#include <algorithm>

#include <assimp/scene.h>

#include "util/assimp_util.h"

#include "Joint.h"

namespace animation {
    bool JointContainer::empty() const noexcept
    {
        return m_joints.empty();
    }

    //void JointContainer::sort() noexcept
    //{
    //    // Sort by nodeIndex to reduce random access in nodes
    //    std::sort(
    //        m_joints.begin(),
    //        m_joints.end(),
    //        [](const auto& a, const auto& b) { return a.m_nodeIndex < b.m_nodeIndex; });
    //}

    Joint& JointContainer::registerJoint(const aiBone* bone) noexcept
    {
        int16_t index;

        std::string nodeName{ bone->mName.C_Str() };
        const auto& it = m_nodeNameToIndex.find(nodeName);
        if (it != m_nodeNameToIndex.end()) {
            index = it->second;
        }
        else {
            index = static_cast<int16_t>(m_nodeNameToIndex.size());
            auto& bi = m_joints.emplace_back(bone);
            bi.m_index = index;

            m_nodeNameToIndex.insert({ nodeName, index });
        }

        return m_joints[index];
    }

    void JointContainer::bindNode(int16_t jointIndex, int16_t nodeIndex) noexcept
    {
        auto& bi = m_joints[jointIndex];
        bi.m_nodeIndex = nodeIndex;
        m_nodeToJoint.insert({ nodeIndex, jointIndex});
    }

    const animation::Joint* JointContainer::getJoint(int16_t jointIndex) const noexcept
    {
        return jointIndex >= 0 ? &m_joints[jointIndex] : nullptr;
    }

    const animation::Joint* JointContainer::findByNodeIndex(int16_t nodeIndex) const noexcept
    {
        const auto& it = m_nodeToJoint.find(nodeIndex);
        return it != m_nodeToJoint.end() ? &m_joints[it->second] : nullptr;
    }
}
