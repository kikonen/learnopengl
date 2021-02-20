#include "InstancedController.h"

#include "model/InstancedNode.h"

InstancedController::InstancedController(const Assets& assets)
	: NodeController(assets)
{
}

InstancedController::~InstancedController()
{
}


void InstancedController::prepare(Node& node) {
	prepareInstanced(dynamic_cast<InstancedNode&>(node));
}

bool InstancedController::update(const RenderContext& ctx, Node& node) {
	return updateInstanced(ctx, dynamic_cast<InstancedNode&>(node));
}

void InstancedController::prepareInstanced(InstancedNode& node)
{
}

bool InstancedController::updateInstanced(const RenderContext& ctx, InstancedNode& node)
{
	return false;
}

