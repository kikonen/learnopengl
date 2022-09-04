#include "InstancedNode.h"

InstancedNode::InstancedNode(NodeType* type, NodeController* controller)
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

	modelBatch.prepare(type);
	selectedBatch.prepare(type);

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

void InstancedNode::bind(const RenderContext& ctx, std::shared_ptr<Shader> shader)
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
	std::shared_ptr<Shader> shader = type->boundShader;
	if (shader->selection) {
		selectedBatch.flush(ctx, type);
	}
	else {
		modelBatch.flush(ctx, type);
	}
}
