#include "InstancedUpdater.h"

#include "InstancedNode.h"

InstancedUpdater::InstancedUpdater(const Assets& assets)
	: NodeUpdater(assets)
{
}

InstancedUpdater::~InstancedUpdater()
{
}


void InstancedUpdater::prepare(Node& node) {
	prepareInstanced(dynamic_cast<InstancedNode&>(node));
}

bool InstancedUpdater::update(const RenderContext& ctx, Node& node) {
	return updateInstanced(ctx, dynamic_cast<InstancedNode&>(node));
}

void InstancedUpdater::prepareInstanced(InstancedNode& node)
{
}

bool InstancedUpdater::updateInstanced(const RenderContext& ctx, InstancedNode& node)
{
	return false;
}

