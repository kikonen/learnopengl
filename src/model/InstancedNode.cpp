#include "InstancedNode.h"

InstancedNode::InstancedNode(std::shared_ptr<NodeType> type)
    : Node(type)
{
}

InstancedNode::~InstancedNode()
{
}

void InstancedNode::prepare(const Assets& assets) noexcept
{
    if (m_prepared) return;

    Node::prepare(assets);

    modelBatch.staticBuffer = true;
    selectedBatch.staticBuffer = true;

    modelBatch.prepare(*m_type.get());
    // TODO KI selectedBatch has been broken for a long time
    // => buffer conflict with modelBatch (overrides IT!!)
    // => corrupting rendering! (rendering from non-initialized buffer) 
    //selectedBatch.prepare(type.get());

    m_buffersDirty = false;
}

void InstancedNode::updateBuffers(const RenderContext& ctx) noexcept
{
    int size = modelBatch.size();
    if (size == 0) return;

    if (modelBatch.staticDrawCount > 0)
        modelBatch.update(modelBatch.staticDrawCount);

    if (selectedBatch.staticDrawCount > 0)
        selectedBatch.update(selectedBatch.staticDrawCount);

    m_buffersDirty = false;
}

void InstancedNode::update(
    const RenderContext& ctx,
    Node* parent) noexcept
{
    Node::update(ctx, parent);
    if (m_buffersDirty) {
        updateBuffers(ctx);
    }
}

void InstancedNode::bind(const RenderContext& ctx, Shader* shader) noexcept
{
    Node::bind(ctx, shader);

    if (shader->m_selection) {
        selectedBatch.bind(ctx, shader);
    }
    else {
        modelBatch.bind(ctx, shader);
    }
}

void InstancedNode::draw(const RenderContext& ctx) noexcept
{
    auto& shader = m_type->m_boundShader;
    modelBatch.flush(ctx, *m_type.get());
}

void InstancedNode::markBuffersDirty() noexcept
{
    m_buffersDirty = true;
}

