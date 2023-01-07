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
    batch.addAll(ctx, m_entityIndeces);
}

void InstancedNode::clear()
{
    m_entityIndeces.clear();
}

void InstancedNode::addEntity(int entityIndex) noexcept
{
    m_entityIndeces.push_back(entityIndex);
}
