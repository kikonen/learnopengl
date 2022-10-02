#pragma once

#include "asset/MeshBuffers.h"

#include "Node.h"

class InstancedNode final : public Node
{
public:
    InstancedNode(std::shared_ptr<NodeType> type);
    virtual ~InstancedNode();

    void prepare(const Assets& assets) override;

    void updateBuffers(const RenderContext& ctx);

    void update(const RenderContext& ctx, Node* parent) override;
    void bind(const RenderContext& ctx, Shader* shader) override;
    void draw(const RenderContext& ctx) override;

    void markBuffersDirty();

private:

public:
    Batch modelBatch;
    Batch selectedBatch;

protected:
    bool m_buffersDirty = true;

private:
    MeshBuffers selectedBuffers;

};

