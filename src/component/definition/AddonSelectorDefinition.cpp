#include "AddonSelectorDefinition.h"

#include "model/Node.h"
#include "model/NodeType.h"

void AddonSelectorDefinition::selectAddons(
    const model::NodeType* type,
    model::Node* node)
{
    const auto* container = type->getAddonMeshContainer();
    if (!container) return;

    for (const auto& addon : addons) {
    }
}
