#pragma once

#include <string>
#include <vector>
#include <map>

#include "ki/size.h"

struct aiBone;

namespace animation {
    struct Joint;

    // Manage joints shared based into their node name
    struct JointContainer {
        bool empty() const noexcept;

        // NOTE KI sorting breaks m_index references
        //void sort() noexcept;

        animation::Joint& registerJoint(const aiBone* bone) noexcept;
        void bindNode(int16_t jointIndex, int16_t nodeIndex) noexcept;

        const animation::Joint* getJoint(int16_t jointIndex) const noexcept;

        // @return Joint, null if not found
        const animation::Joint* findByNodeIndex(int16_t nodeIndex) const noexcept;

        inline bool hasJoints() const noexcept {
            return !m_nodeNameToIndex.empty();
        }

        inline int16_t size() const noexcept {
            return static_cast<int16_t>(m_nodeNameToIndex.size());
        }

        std::vector<animation::Joint> m_joints;
        std::map<std::string, int16_t> m_nodeNameToIndex;

        std::map<int16_t, int16_t> m_nodeToJoint;
    };
}
