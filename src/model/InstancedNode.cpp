#include "InstancedNode.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"

InstancedNode::InstancedNode(MeshType* type)
    : Node(type)
{
}

InstancedNode::~InstancedNode()
{
}

void InstancedNode::prepare(
    const Assets& assets,
    EntityRegistry& entityRegistry)
{
    if (m_prepared) return;

    Node::prepare(assets, entityRegistry);
}

void InstancedNode::updateBuffers(const RenderContext& ctx) noexcept
{
}

void InstancedNode::update(
    const RenderContext& ctx,
    Node* parent) noexcept
{
    Node::update(ctx, parent);
}

void InstancedNode::bindBatch(const RenderContext& ctx, Batch& batch) noexcept
{
    batch.addInstanced(ctx, m_firstIndex, m_count);
}

void InstancedNode::setEntityRange(int firstIndex, int count) noexcept
{
    m_firstIndex = firstIndex;
    m_count = count;
}
