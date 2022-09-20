#include "InstancedController.h"

#include "model/InstancedNode.h"

InstancedController::InstancedController()
{
}

void InstancedController::prepare(const Assets& assets, Node& node) {
    prepareInstanced(assets, dynamic_cast<InstancedNode&>(node));
}

bool InstancedController::update(const RenderContext& ctx, Node& node) {
    return updateInstanced(ctx, dynamic_cast<InstancedNode&>(node));
}

void InstancedController::prepareInstanced(const Assets& assets, InstancedNode& node)
{
}

bool InstancedController::updateInstanced(const RenderContext& ctx, InstancedNode& node)
{
    return false;
}

