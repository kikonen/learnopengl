#include "JointContainer.h"

#include <algorithm>

#include <assimp/scene.h>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/glm_format.h"
#include "util/assimp_util.h"

#include "Joint.h"

namespace animation {
    bool JointContainer::empty() const noexcept
    {
        return m_joints.empty();
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
                        "offset_matrix_mismatch: node={}.{}, old={}.{}, new={}.{}",
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
