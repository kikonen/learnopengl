#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "asset/Material.h"

#include "RigNode.h"
#include "BoneContainer.h"

struct aiNode;

namespace animation {
    struct Animation;
    struct RigNode;

    struct RigContainer {
        ~RigContainer();

        animation::RigNode& addNode(const aiNode* node);

        void addAnimation(std::unique_ptr<animation::Animation> animation);

        const animation::RigNode* getNode(int16_t index) const noexcept;

        // @return nullptr if not found
        const animation::RigNode* findNode(const std::string& name) const noexcept;

        bool hasBones() const noexcept;

        // @return registered index in m_sockets
        uint16_t addSocket(const RigNode& rigNode);
        bool hasSockets() const noexcept;

        void prepare();
        void validate() const;

        //void calculateInvTransforms() noexcept;

        std::vector<animation::RigNode> m_nodes;

        BoneContainer m_boneContainer;

        std::vector<uint16_t> m_sockets;

        std::vector<std::unique_ptr<animation::Animation>> m_animations;
    };
}
