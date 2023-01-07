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

void InstancedNode::prepare(const Assets& assets) noexcept
{
    if (m_prepared) return;

    Node::prepare(assets);
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
    batch.addAll(ctx, m_modelMatrices, m_normalMatrices, m_objectIDs, getHighlightColor(ctx));
}

void InstancedNode::clear()
{
    m_modelMatrices.clear();
    m_normalMatrices.clear();
    m_objectIDs.clear();
}

void InstancedNode::add(
    const glm::mat4& model,
    const glm::mat3& normal,
    int objectID) noexcept
{
    m_modelMatrices.push_back(model);
    m_normalMatrices.push_back(normal);
    m_objectIDs.push_back(objectID);
}
