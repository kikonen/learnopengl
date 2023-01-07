#pragma once

#include "Node.h"

class InstancedNode final : public Node
{
public:
    InstancedNode(MeshType* type);
    virtual ~InstancedNode();

    void prepare(
        const Assets& assets,
        EntityRegistry& entityRegistry) override;

    void updateBuffers(const RenderContext& ctx) noexcept;

    virtual void update(const RenderContext& ctx, Node* parent) noexcept override;

    virtual void bindBatch(const RenderContext& ctx, Batch& batch) noexcept override;

    void clear();

    void addEntity(int entityIndex) noexcept;

private:
    std::vector<int> m_entityIndeces;
};
