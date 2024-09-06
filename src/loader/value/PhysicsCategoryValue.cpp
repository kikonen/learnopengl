#include "PhysicsCategoryValue.h"

#include <unordered_map>

#include <ode/ode.h>

#include "util/debug.h"

#include "physics/Category.h"
#include "physics/physics_util.h"

#include "loader/document.h"
#include "loader/loader_util.h"

namespace {
    std::unordered_map<std::string, physics::Category> g_categoryMapping;
    std::unordered_map<std::string, uint32_t> g_maskMapping;
    std::unordered_map<std::string, uint32_t> g_contactMapping;

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
                // ray
                { "ray_player_fire", physics::Category::ray_player_fire },
                { "ray_npc_fire", physics::Category::ray_npc_fire },
                { "ray_hit", physics::Category::ray_hit },
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
                    physics::Category::npc) },
                { "ray", physics::mask(
                    physics::Category::ray_player_fire,
                    physics::Category::ray_npc_fire,
                    physics::Category::ray_hit) },
                });
        }
        return g_maskMapping;
    }

    const std::unordered_map<std::string, uint32_t>& getContactMapping()
    {
        if (g_contactMapping.empty()) {
            g_contactMapping.insert({
                { "mu2", dContactMu2 },
                { "fdir1", dContactFDir1 },
                { "bounce", dContactBounce },
                { "soft_erp", dContactSoftERP },
                { "soft_cfm", dContactSoftCFM },
                { "motion1", dContactMotion1 },
                { "motion2", dContactMotion2 },
                { "motionn", dContactMotionN },
                { "slip1", dContactSlip1 },
                { "slip2", dContactSlip2 },
                { "rolling", dContactRolling },
                { "approx1_!", dContactApprox1_1 },
                { "approax1_2", dContactApprox1_2 },
                { "approx1_n", dContactApprox1_N },
                { "approx1", dContactApprox1 },
                });
        }
        return g_contactMapping;
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
