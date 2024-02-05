#include "NodeController.h"

#include "engine/PrepareContext.h"

NodeController::NodeController() = default;

NodeController::~NodeController() = default;

void NodeController::prepare(
    const PrepareContext& ctx,
    Node& node)
{
    if (m_prepared) return;
    m_prepared = true;
}
