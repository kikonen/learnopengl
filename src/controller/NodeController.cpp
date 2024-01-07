#include "NodeController.h"

#include "engine/PrepareContext.h"

void NodeController::prepare(
    const PrepareContext& ctx,
    Node& node)
{
    if (m_prepared) return;
    m_prepared = true;

    m_registry = ctx.m_registry;
}
