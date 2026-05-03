#include "AddonSelectorDefinition.h"

#include <algorithm>
#include <vector>

#include "ki/sid.h"

#include "util/Random.h"

#include "mesh/LodMesh.h"
#include "mesh/LodMeshContainer.h"

#include "model/Node.h"
#include "model/NodeType.h"

namespace {
    constexpr uint32_t SEED_MIX{ 0x9E3779B9u };

    const util::Ref<mesh::LodMesh>* findAddonById(
        const std::vector<util::Ref<mesh::LodMesh>>& pool,
        ki::sid_t id) noexcept
    {
        for (const auto& ref : pool) {
            if (ref->getId() == id) return &ref;
        }
        return nullptr;
    }

    std::vector<size_t> collectGroupCandidates(
        const std::vector<util::Ref<mesh::LodMesh>>& pool,
        ki::sid_t group)
    {
        std::vector<size_t> out;
        for (size_t i = 0; i < pool.size(); ++i) {
            if (pool[i]->getGroupId() == group) out.push_back(i);
        }
        return out;
    }

    void selectFromGroup(
        const AddonDefinition& addon,
        const model::Node* node,
        const std::vector<util::Ref<mesh::LodMesh>>& pool,
        std::vector<util::Ref<mesh::LodMesh>>& enabled)
    {
        auto candidates = collectGroupCandidates(pool, addon.group);
        if (candidates.empty()) return;

        const uint32_t maxN = std::min<uint32_t>(addon.range.y, static_cast<uint32_t>(candidates.size()));
        const uint32_t minN = std::min<uint32_t>(addon.range.x, maxN);

        if (addon.seed == 0) {
            for (uint32_t i = 0; i < minN; ++i) {
                enabled.push_back(pool[candidates[i]]);
            }
            return;
        }

        const int seed = static_cast<int>(addon.seed * SEED_MIX ^ node->getId());
        util::Random rng{ seed };

        const uint32_t span = maxN - minN + 1;
        uint32_t count = minN + static_cast<uint32_t>(rng.rnd() * span);
        if (count > maxN) count = maxN;

        // Fisher-Yates partial shuffle: bring `count` random picks to the front
        for (uint32_t i = 0; i < count; ++i) {
            const size_t remaining = candidates.size() - i;
            size_t j = i + static_cast<size_t>(rng.rnd() * remaining);
            if (j >= candidates.size()) j = candidates.size() - 1;
            std::swap(candidates[i], candidates[j]);
        }

        for (uint32_t i = 0; i < count; ++i) {
            enabled.push_back(pool[candidates[i]]);
        }
    }
}

void AddonSelectorDefinition::selectAddons(
    const model::NodeType* type,
    model::Node* node)
{
    auto& enabled = node->m_enabledMeshes;

    if (const auto* base = type->getMeshContainer()) {
        for (const auto& ref : base->getLodMeshes()) {
            enabled.push_back(ref);
        }
    }

    const auto* addonContainer = type->getAddonMeshContainer();
    if (!addonContainer) return;

    const auto& pool = addonContainer->getLodMeshes();

    for (const auto& addon : addons) {
        if (!addon.enabled) continue;

        if (addon.id != 0) {
            if (const auto* match = findAddonById(pool, addon.id)) {
                enabled.push_back(*match);
            }
            continue;
        }

        if (addon.group != 0) {
            selectFromGroup(addon, node, pool, enabled);
        }
    }
}
