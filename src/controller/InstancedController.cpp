#include "InstancedController.h"

#include "model/InstancedNode.h"

InstancedController::InstancedController()
{
}

void InstancedController::prepare(const Assets& assets, Node& node) {
    if (m_prepared) return;
    m_prepared = true;

    prepareInstanced(assets, dynamic_cast<InstancedNode&>(node));
}

bool InstancedController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent)
{
    return updateInstanced(ctx, dynamic_cast<InstancedNode&>(node), parent);
}

void InstancedController::prepareInstanced(const Assets& assets, InstancedNode& node)
{
}

bool InstancedController::updateInstanced(
    const RenderContext& ctx,
    InstancedNode& node,
    Node* parent)
{
    return false;
}

