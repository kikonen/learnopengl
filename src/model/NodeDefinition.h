#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "NodeRepeat.h"

namespace model
{
    // For defining composite types
    struct NodeDefinition
    {
        bool m_enabled;

        std::string m_id;
        std::string m_parentId;
        std::string m_aliasId;
        std::string m_ignoredById;

        std::string m_socketId;

        std::string m_tagId;

        ki::type_id m_typeId;

        glm::vec3 m_position{ 0.f };
        glm::vec3 m_rotation{ 0.f };
        glm::vec3 m_scale{ 1.f };

        // material tiling
        glm::vec2 m_tiling{ 1.f };

        bool m_active{ false };

        NodeRepeat m_repeat;
        glm::vec3 m_clonePositionOffset{ 0.f };

        std::shared_ptr<std::vector<model::NodeDefinition>> m_clones;
        std::shared_ptr<std::vector<model::NodeDefinition>> m_children;
    };
}
