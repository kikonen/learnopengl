#include "NodeController.h"

NodeController::NodeController(const std::shared_ptr<Assets> assets)
    : assets(assets)
{
}

NodeController::~NodeController()
{
}

void NodeController::prepare(Node& node)
{
}

bool NodeController::update(const RenderContext& ctx, Node& node)
{
    return false;
}
