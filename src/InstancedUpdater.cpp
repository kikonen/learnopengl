#include "InstancedUpdater.h"

InstancedUpdater::InstancedUpdater(const Assets& assets)
	: assets(assets)
{
}

InstancedUpdater::~InstancedUpdater()
{
}

void InstancedUpdater::prepare(InstancedNode& node)
{
}

bool InstancedUpdater::update(const RenderContext& ctx, InstancedNode& node)
{
	return false;
}
