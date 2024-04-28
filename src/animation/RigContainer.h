#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "RigNode.h"
#include "BoneContainer.h"
#include "VertexBone.h"

struct aiNode;

namespace animation {
    struct Animation;

    struct RigContainer {
        ~RigContainer();

        animation::RigNode& addNode(const aiNode* node);
        void addAnimation(std::unique_ptr<animation::Animation> animation);

        inline const animation::RigNode* getNode(int16_t id) const noexcept
        {
            return &m_nodes[id];
        }

        int16_t findNodeId(const std::string& name) const noexcept;

        std::vector<animation::RigNode> m_nodes;

        BoneContainer m_bones;
        std::vector<animation::VertexBone> m_vertexBones;

        std::vector<std::unique_ptr<animation::Animation>> m_animations;

        std::map<size_t, ki::material_id> m_materialMapping;
    };
}
