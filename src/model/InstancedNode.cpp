#include "InstancedNode.h"

InstancedNode::InstancedNode(std::shared_ptr<NodeType> type, NodeController* controller)
	: Node(type)
{
	this->controller = controller;
}

InstancedNode::~InstancedNode()
{
}

void InstancedNode::prepare(const Assets& assets)
{
	Node::prepare(assets);

	modelBatch.staticBuffer = true;
	selectedBatch.staticBuffer = true;

	modelBatch.prepare(type.get());
	selectedBatch.prepare(type.get());

	buffersDirty = false;
}

void InstancedNode::updateBuffers(const RenderContext& ctx)
{
	int size = modelBatch.size();
	modelBatch.update(size);
	selectedBatch.update(size);

	buffersDirty = false;
}

bool InstancedNode::update(const RenderContext& ctx)
{
	bool updated = Node::update(ctx);
	buffersDirty = buffersDirty || updated;
	if (buffersDirty) {
		updateBuffers(ctx);
	}
	return updated;
}

void InstancedNode::bind(const RenderContext& ctx, Shader* shader)
{
	Node::bind(ctx, shader);

	if (shader->selection) {
		selectedBatch.bind(ctx, shader);
	}
	else {
		modelBatch.bind(ctx, shader);
	}
}

void InstancedNode::draw(const RenderContext& ctx)
{
	auto shader = type->boundShader;
	if (shader->selection) {
		selectedBatch.flush(ctx, type.get());
	}
	else {
		modelBatch.flush(ctx, type.get());
	}
}
