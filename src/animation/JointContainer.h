#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "ki/size.h"

#include "Joint.h"

struct aiBone;

namespace animation {
    // Manage joints shared based into their node name
    struct JointContainer {
        bool empty() const noexcept;

        inline int16_t size() const noexcept {
            return static_cast<int16_t>(m_joints.size());
        }

        animation::Joint& registerJoint(
            const aiBone* bone,
            int16_t nodeIndex) noexcept;

        // @return Joint, null if not found
        const animation::Joint * findByNodeIndex(int16_t nodeIndex) const noexcept;

        std::vector<animation::Joint> m_joints;
        std::unordered_map<int16_t, int16_t> m_nodeToJoint;
    };
}
