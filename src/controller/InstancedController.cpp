#include "InstancedController.h"

#include "model/InstancedNode.h"

InstancedController::InstancedController()
{
}

void InstancedController::prepare(
    const Assets& assets,
    EntityRegistry& entityRegistry,
    Node& node)
{
    if (m_prepared) return;
    m_prepared = true;

    prepareInstanced(
        assets,
        entityRegistry,
        dynamic_cast<InstancedNode&>(node));
}

bool InstancedController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    return updateInstanced(ctx, dynamic_cast<InstancedNode&>(node), parent);
}

void InstancedController::prepareInstanced(
    const Assets & assets,
    EntityRegistry& entityRegistry,
    InstancedNode& node)
{
}

bool InstancedController::updateInstanced(
    const RenderContext& ctx,
    InstancedNode& node,
    Node* parent)
{
    return false;
}

