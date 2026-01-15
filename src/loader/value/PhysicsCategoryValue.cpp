#include "PhysicsCategoryValue.h"

#include <unordered_map>

#include "util/debug.h"

#include "physics/Category.h"
#include "physics/physics_util.h"

#include "loader/document.h"
#include "loader/loader_util.h"

namespace {
    std::unordered_map<std::string, physics::Category> g_categoryMapping;
    std::unordered_map<std::string, uint32_t> g_maskMapping;

    const std::unordered_map<std::string, physics::Category>& getCategoryMapping()
    {
        if (g_categoryMapping.empty()) {
            g_categoryMapping.insert({
                { "none", physics::Category::none },
                { "all", physics::Category::all },
                { "invalid", physics::Category::invalid },
                // world
                { "terrain", physics::Category::terrain },
                { "ground", physics::Category::ground },
                { "water", physics::Category::water },
                { "scenery", physics::Category::scenery },
                { "prop", physics::Category::prop },
                // characters and creatures
                { "npc", physics::Category::npc },
                { "player", physics::Category::player },
                // characteristic
                { "can_float", physics::Category::can_float },
                { "can_terrain", physics::Category::can_terrain },
                });
        }
        return g_categoryMapping;
    }

    const std::unordered_map<std::string, uint32_t>& getMaskMapping()
    {
        if (g_maskMapping.empty()) {
            g_maskMapping.insert({
                { "world", physics::mask(
                    physics::Category::terrain,
                    // NOTE KI things don't float by default
                    //physics::Category::water,
                    physics::Category::scenery,
                    physics::Category::prop) },
                { "character", physics::mask(
                    physics::Category::player,
                    physics::Category::npc) }
                });
        }
        return g_maskMapping;
    }

    uint32_t readCategoryMask(std::string v)
    {
        {
            const auto& mapping = getCategoryMapping();
            const auto& it = mapping.find(v);
            if (it != mapping.end()) return physics::mask(it->second);
        }
        {
            const auto& mapping = getMaskMapping();
            const auto& it = mapping.find(v);
            if (it != mapping.end()) return it->second;
        }

        // NOTE KI for data tracking data mismatches
        KI_WARN_OUT(fmt::format("PHYSICS: INVALID_CATEGORY_MASK - category={}", v));
        return physics::mask(physics::Category::invalid);
    }
}

namespace loader {
    void PhysicsCategoryValue::loadMask(
        const loader::DocNode& node,
        uint32_t& result) const
    {
        uint32_t mask = 0;
        std::vector<std::string> negatedEntries;

        for (const auto& entry : node.getNodes()) {
            const auto& value = readString(entry);
            if (value[0] == '_') {
                negatedEntries.push_back(value.substr(1, value.length() - 1));
            }
            else {
                mask |= readCategoryMask(value);
            }
        }

        if (!negatedEntries.empty()) {
            if (mask == 0) mask = UINT_MAX;
            for (const auto& value : negatedEntries) {
                mask &= ~readCategoryMask(value);
            }
        }

        KI_INFO_OUT(fmt::format("PHYSICS: MASK={}, nodes={}", mask, renderNodes(node.getNodes())));

        result = mask;
    }
}
