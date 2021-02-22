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

	modelBatch.size = 1000;
	selectedBatch.size = 1000;

	modelBatch.clearBuffer = false;
	selectedBatch.clearBuffer = false;

	modelBatch.prepare(type);
	selectedBatch.prepare(type);

	buffersDirty = false;
}

void InstancedNode::updateBuffers(const RenderContext& ctx)
{
	int size = modelBatch.modelMatrices.size();
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

Shader* InstancedNode::bind(const RenderContext& ctx, Shader* shader)
{
	shader = Node::bind(ctx, shader);

	if (shader->selection) {
		selectedBatch.bind(ctx, shader);
	}
	else {
		modelBatch.bind(ctx, shader);
	}

	return shader;
}

void InstancedNode::draw(const RenderContext& ctx)
{
	Shader* shader = type->boundShader;
	if (shader->selection) {
		selectedBatch.flush(ctx, type);
	}
	else {
		modelBatch.flush(ctx, type);
	}
}
