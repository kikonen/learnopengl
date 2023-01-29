#include "NodeController.h"

#include "model/Node.h"

NodeController::NodeController()
{
}

void NodeController::prepare(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    if (m_prepared) return;
    m_prepared = true;
}

bool NodeController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    return false;
}
