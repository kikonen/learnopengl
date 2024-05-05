#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "RigNode.h"
#include "BoneContainer.h"

struct aiNode;

namespace animation {
    struct Animation;
    struct RigNode;
    struct VertexBone;

    struct RigContainer {
        ~RigContainer();

        animation::RigNode& addNode(const aiNode* node);

        void addAnimation(std::unique_ptr<animation::Animation> animation);

        const animation::RigNode* getNode(int16_t index) const noexcept;

        // @return nullptr if not found
        const animation::RigNode* findNode(const std::string& name) const noexcept;

        bool hasBones() const noexcept;

        void calculateInvTransforms() noexcept;

        std::vector<animation::RigNode> m_nodes;

        BoneContainer m_boneContainer;
        std::vector<animation::VertexBone> m_vertexBones;

        std::vector<std::unique_ptr<animation::Animation>> m_animations;

        std::map<size_t, ki::material_id> m_materialMapping;
    };
}
