#include "InstancedNode.h"

InstancedNode::InstancedNode(std::shared_ptr<NodeType> type)
    : Node(type)
{
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

    m_buffersDirty = false;
}

void InstancedNode::updateBuffers(const RenderContext& ctx)
{
    int size = modelBatch.size();
    if (size == 0) return;

    modelBatch.update(size);
    selectedBatch.update(size);

    m_buffersDirty = false;
}

void InstancedNode::update(
    const RenderContext& ctx,
    Node* parent)
{
    Node::update(ctx, parent);
    if (m_buffersDirty) {
        updateBuffers(ctx);
    }
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
    auto& shader = type->boundShader;
    if (shader->selection) {
        selectedBatch.flush(ctx, type.get());
    }
    else {
        modelBatch.flush(ctx, type.get());
    }
}
