#include "JointContainer.h"

#include <algorithm>

#include <assimp/scene.h>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/util_join.h"
#include "util/glm_format.h"
#include "util/assimp_util.h"

#include "Rig.h"
#include "RigNode.h"
#include "Joint.h"

namespace animation {
    bool JointContainer::empty() const noexcept
    {
        return m_joints.empty();
    }

    void JointContainer::dump() const
    {
        //KI_INFO_OUT(fmt::format(
        //    "\n=======================\n[RIG SUMMARY: {} ({})]\nHIERARCHY:\n{}\nANIMATIONS:\n{}\nSOCKETS:\n{}\n=======================",
        //    m_name,
        //    m_skeletonRootNodeName,
        //    getHierarchySummary(0),
        //    getAnimationSummary(0),
        //    getSocketSummary(0)));
    }

    void JointContainer::validate() const
    {
        std::vector<const animation::Joint*> unboundJoints;

        // NOTE KI check that all joints are related to some node
        // - every joint has node
        // - not every node has joint
        for (const auto& joint : m_joints) {
            if (joint.m_nodeIndex >= 0) continue;

            unboundJoints.push_back(&joint);
        }

        if (!unboundJoints.empty()) {
            auto sb = util::join(
                unboundJoints, ", ",
                [](const auto* joint) {
                return joint->m_nodeName;
            });

            KI_WARN_OUT(fmt::format(
                "ANIM::RIG::MISSING_JOINT_NODES: container={}, missing=[{}]",
                m_name, sb));
        }
    }

    void JointContainer::bindRig(animation::Rig& rig)
    {
        m_nodeToJoint.clear();

        for (auto& joint : m_joints) {
            joint.m_nodeIndex = -1;

            auto* rigNode = rig.findNode(joint.m_nodeName);
            if (!rigNode) {
                KI_WARN_OUT(fmt::format(
                    "ANIM::RIG::MISSING_JOINT_NODE: container={}, node={}, index={}",
                    m_name, joint.m_nodeName, joint.m_jointIndex));
                continue;
            }

            joint.m_nodeIndex = rigNode->m_index;
            m_nodeToJoint.insert({ rigNode->m_index, joint.m_jointIndex });
            rigNode->m_hasJoint = true;
        }

        validate();
    }

    animation::Joint* JointContainer::registerJoint(
        const aiBone* bone)
    {
        auto jointIndex = static_cast<int16_t>(m_joints.size());
        return &m_joints.emplace_back(bone, jointIndex);
    }

    const animation::Joint* JointContainer::findByNodeIndex(int16_t nodeIndex) const noexcept
    {
        const auto& it = m_nodeToJoint.find(nodeIndex);
        return it != m_nodeToJoint.end() ? &m_joints[it->second] : nullptr;
    }
}
