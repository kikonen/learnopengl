#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "ki/size.h"

#include "Joint.h"

struct aiBone;

namespace animation {
    struct Rig;

    // Manage joints shared based into their node name
    //
    // TODO KI need to change logic so that mesh_set can share
    // same rig (and thus animate nodes only ONCE), but have separate
    // joint containers *per mesh*
    // => Makes huge difference especially with LODx meshes sharing same skeleton
    struct JointContainer {
        const std::string m_name;
        std::vector<animation::Joint> m_joints;
        std::unordered_map<int16_t, int16_t> m_nodeToJoint;

        JointContainer(const std::string& name)
            : m_name{ name }
        {
        }

        bool empty() const noexcept;

        inline int16_t size() const noexcept {
            return static_cast<int16_t>(m_joints.size());
        }

        void dump() const;
        void validate() const;

        void bindRig(animation::Rig& rig);

        animation::Joint* registerJoint(
            const aiBone* bone);

        // @return Joint, null if not found
        const animation::Joint * findByNodeIndex(int16_t nodeIndex) const noexcept;

        private:
            animation::Joint& registerJoint(
                const aiBone* bone,
                int16_t nodeIndex);
    };
}
