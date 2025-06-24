#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

#include <glm/glm.hpp>

#include "BaseId.h"
#include "BaseData.h"
#include "Repeat.h"

namespace loader {
    struct NodeData {
        bool enabled{ false };
        bool active{ false };

        BaseId baseId;
        BaseId parentBaseId;
        BaseId ignoredByBaseId;

        BaseId typeId;

        std::string name;
        std::string desc;

        glm::vec3 position{ 0.f };
        glm::vec3 rotation{ 0.f };
        glm::vec3 scale{ 1.f };

        bool selected{ false };
        bool shareType{ true };

        glm::vec3 clonePositionOffset{ 0.f };

        Repeat repeat;

        // material tiling
        float tilingX{ 1.f };
        float tilingY{ 1.f };

        std::shared_ptr<std::vector<NodeData>> clones;
        std::shared_ptr<std::vector<NodeData>> children;

        const std::string& str() const noexcept
        {
            return baseId.str();
        }
    };
}
