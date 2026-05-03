#include "AddonSelectorDefinition.h"

#include <fmt/format.h>

#include "ki/sid.h"

#include "mesh/LodMesh.h"
#include "mesh/LodMeshContainer.h"
#include "mesh/Mesh.h"
#include "mesh/LodMeshInstance.h"

#include "model/Node.h"
#include "model/NodeType.h"

namespace {
    const util::Ref<mesh::LodMesh>* findAddon(
        const std::vector<util::Ref<mesh::LodMesh>>& pool,
        ki::sid_t id) noexcept
    {
        for (const auto& ref : pool) {
            //const auto* mesh = ref->getMesh<mesh::Mesh>();
            //if (!mesh) continue;
            //ki::sid_t meshId = SID(mesh->m_alias);
            if (ref->getId() == id) return &ref;
        }
        return nullptr;
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
    if (addonContainer) {
        for (const auto& addon : addons) {
            if (!addon.enabled) continue;

            KI_INFO_OUT(fmt::format("ADDON: id={}, addon={}", addon.id, SID_NAME(addon.id)));

            const auto* match = findAddon(addonContainer->getLodMeshes(), addon.id);
            if (!match) continue;

            enabled.push_back(*match);
        }
    }

    //node->m_lodMeshInstances.resize(enabled.size());
}
