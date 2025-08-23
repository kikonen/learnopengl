#include "NodeController.h"

#include "engine/PrepareContext.h"

NodeController::NodeController(
    bool pawn,
    bool camera)
    : m_pawn{ pawn },
    m_camera{ camera }
{ }

NodeController::~NodeController() = default;

void NodeController::prepare(
    const PrepareContext& ctx,
    Node& node)
{
    if (m_prepared) return;
    m_prepared = true;

    m_registry = ctx.m_registry;
}
