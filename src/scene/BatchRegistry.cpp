#include "BatchRegistry.h"

#include "model/Node.h"

BatchRegistry::BatchRegistry(const Assets& assets)
    : m_assets(assets)
{
}

Batch*BatchRegistry::getBatch(int vao)
{
    return &m_batches[vao];
}

void BatchRegistry::prepare()
{
}

void BatchRegistry::prepareVAO(
    GLVertexArray& vao,
    bool singleMaterial)
{
    auto* batch = getBatch(vao);
    batch->prepare(m_assets, m_assets.batchSize);
    batch->prepareVAO(vao, singleMaterial);
}

void BatchRegistry::draw(
    const RenderContext& ctx,
    Node& node,
    Shader* shader)
{
    auto* vao = node.m_type->m_vao;
    if (!vao) return;

    auto* batch = getBatch(vao->id);
    if (batch != m_current) {
        if (m_current) {
            m_current->flush(ctx, false);
        }
        m_current = batch;
        m_current->bind();
    }

    m_current->draw(ctx, node, shader);
}

void BatchRegistry::flush(
    const RenderContext& ctx,
    bool release)
{
    if (!m_current) return;
    m_current->flush(ctx, release);
}
