#include "InstancedController.h"

#include "model/Node.h"

InstancedController::InstancedController()
{
}

void InstancedController::prepare(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    if (m_prepared) return;
    m_prepared = true;

    prepareInstanced(
        assets,
        registry,
        node);
}

bool InstancedController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    return updateInstanced(ctx, node, parent);
}

void InstancedController::prepareInstanced(
    const Assets & assets,
    Registry* registry,
    Node& node)
{
}

bool InstancedController::updateInstanced(
    const RenderContext& ctx,
    Node& node,
    Node* parent)
{
    return false;
}

