#include "InstancedNode.h"

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

void InstancedNode::draw(const RenderContext& ctx) noexcept
{
    ctx.m_batch.drawAll(ctx, m_type, m_modelMatrices, m_normalMatrices, m_objectIDs);
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
