#include "NodeUpdater.h"

NodeUpdater::NodeUpdater(const Assets& assets)
	: assets(assets)
{
}

NodeUpdater::~NodeUpdater()
{
}

void NodeUpdater::prepare(Node& node)
{
}

bool NodeUpdater::update(const RenderContext& ctx, Node& node)
{
	return false;
}
