#pragma once

#include "asset/MeshBuffers.h"

#include "Node.h"

class InstancedNode final : public Node
{
public:
    InstancedNode(std::shared_ptr<NodeType> type);
    virtual ~InstancedNode();

    void prepare(const Assets& assets) noexcept override;

    void updateBuffers(const RenderContext& ctx) noexcept;

    void update(const RenderContext& ctx, Node* parent) noexcept override;
    void bind(const RenderContext& ctx, Shader* shader) noexcept override;
    void draw(const RenderContext& ctx) noexcept override;

    void markBuffersDirty() noexcept;

private:

public:
    Batch modelBatch;
    Batch selectedBatch;

protected:
    bool m_buffersDirty = true;

private:
    MeshBuffers selectedBuffers;

};

