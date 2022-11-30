#pragma once

#include "asset/MeshBuffers.h"

#include "Node.h"

class InstancedNode final : public Node
{
public:
    InstancedNode(MeshType* type);
    virtual ~InstancedNode();

    void prepare(const Assets& assets) noexcept override;

    void updateBuffers(const RenderContext& ctx) noexcept;

    void update(const RenderContext& ctx, Node* parent) noexcept override;
    void bind(const RenderContext& ctx, Shader* shader) noexcept override;
    void draw(const RenderContext& ctx) noexcept override;

    void clear();

    void add(
        const glm::mat4& model,
        const glm::mat3& normal,
        int objectID) noexcept;

private:
    std::vector<glm::mat4> m_modelMatrices;
    std::vector<glm::mat3> m_normalMatrices;
    std::vector<int> m_objectIDs;
};

