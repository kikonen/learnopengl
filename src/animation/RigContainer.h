#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "asset/Material.h"

#include "RigJoint.h"
#include "RigSocket.h"

#include "BoneContainer.h"

struct aiNode;
struct aiBone;

namespace animation {
    struct Animation;
    struct RigJoint;

    struct RigContainer {
        ~RigContainer();

        animation::RigJoint& addJoint(const aiNode* node);

        // Register new bone or return old
        animation::BoneInfo& registerBone(const aiBone* bone) noexcept;

        // @return true if rig is empty (thus no bones, and thus rig is not needed)
        bool empty() const noexcept
        {
            return m_boneContainer.empty();
        }

        void addAnimation(std::unique_ptr<animation::Animation> animation);

        const animation::RigJoint* getJoint(int16_t index) const noexcept;

        // @return nullptr if not found
        const animation::RigJoint* findJoint(const std::string& name) const noexcept;

        bool hasBones() const noexcept;

        // @return registered index in m_sockets, -1 if joint not found
        int16_t registerSocket(const animation::RigSocket& socket);

        // @return registered socket, null if not found
        const animation::RigSocket* getSocket(int16_t socketIndex) const noexcept;

        bool hasSockets() const noexcept
        {
            return !m_sockets.empty();
        }

        void prepare();
        void validate() const;

        //void calculateInvTransforms() noexcept;

        std::vector<animation::RigJoint> m_joints;

        BoneContainer m_boneContainer;

        std::vector<animation::RigSocket> m_sockets;

        std::map<std::string, uint16_t> m_NameToSocket;

        std::vector<std::unique_ptr<animation::Animation>> m_animations;
    };
}
