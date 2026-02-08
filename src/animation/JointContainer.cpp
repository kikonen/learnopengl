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

        if (unboundJoints.empty()) {
            auto sb = util::join(
                unboundJoints, ", ",
                [](const auto* joint) {
                return joint->m_nodeName;
            });

            throw std::runtime_error(fmt::format(
                "ANIM::RIG::MISSING_JOINT_NODES: {}",
                sb));
        }
    }

    animation::Joint* JointContainer::registerJoint(
        const aiBone* bone)
    {
        std::string nodeName = assimp_util::normalizeName(bone->mName);
        auto* rigNode = m_rig->findNode(nodeName);

        if (!rigNode) {
            //throw fmt::format("ANIM::RIG_NODE_NOT_FOUND: joint={}", nodeName);
            KI_ERROR(fmt::format(
                "ANIM::RIG::NODE_NOT_FOUND: rig={}, joint={}",
                m_name, nodeName));
            //return nullptr;
        }

        if (rigNode) {
            rigNode->m_hasJoint = true;
        }

        return &registerJoint(bone, rigNode ? rigNode->m_index : -1);
    }

    Joint& JointContainer::registerJoint(
        const aiBone* bone,
        int16_t nodeIndex)
    {
        auto jointIndex = static_cast<int16_t>(m_joints.size());

        {
            const auto& it = m_nodeToJoint.find(nodeIndex);
            if (it != m_nodeToJoint.end()) {
                auto& oldJoint = m_joints[it->second];
                Joint newJoint{ bone, jointIndex, nodeIndex };

                const auto& oldMatrix = fmt::format("{}", oldJoint.m_offsetMatrix);
                const auto& newMatrix = fmt::format("{}", newJoint.m_offsetMatrix);

                if (oldMatrix != newMatrix) {
                    throw fmt::format(
                        "ANIM::OFFSET_MATRIX_MISMATCH: node={}.{}, old={}.{}, new={}.{}",
                        nodeIndex, oldJoint.m_nodeName,
                        oldJoint.m_jointIndex, oldJoint.m_offsetMatrix,
                        jointIndex, newJoint.m_offsetMatrix);
                }
                return oldJoint;
            }
        }

        // NOTE KI in the context of single container there should not be conflicts
        if (nodeIndex >= 0) {
            m_nodeToJoint.insert({nodeIndex, jointIndex});
        }

        return m_joints.emplace_back(bone, jointIndex, nodeIndex);
    }

    const animation::Joint* JointContainer::findByNodeIndex(int16_t nodeIndex) const noexcept
    {
        const auto& it = m_nodeToJoint.find(nodeIndex);
        return it != m_nodeToJoint.end() ? &m_joints[it->second] : nullptr;
    }
}
