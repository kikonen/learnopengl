#pragma once

#include "scene/RenderContext.h"
#include "asset/Shader.h"

class NodeType;
class Node;

class Batch final
{
public:
    Batch();
    ~Batch();

    void add(const glm::mat4& model, const glm::mat3& normal, int objectID) noexcept;
    void reserve(size_t count) noexcept;
    int size() noexcept;

    void clear() noexcept;

    void prepare(NodeType& type) noexcept;

    void update(size_t count) noexcept;
    void bind(const RenderContext& ctx, Shader* shader) noexcept;
    void draw(const RenderContext& ctx, Node& node, Shader* shader) noexcept;

    void flush(const RenderContext& ctx, const NodeType& type) noexcept;

public:
    int batchSize = -1;

    bool staticBuffer = false;
    int staticDrawCount = 0;

    bool dirty = false;
    bool objectIDBuffer = false;

private:
    bool m_prepared = false;

    std::vector<glm::mat4> m_modelMatrices;
    std::vector<glm::mat3> m_normalMatrices;
    std::vector<glm::vec4> m_objectIDs;

    unsigned int m_modelBufferId = 0;
    unsigned int m_normalBufferId = 0;
    unsigned int m_objectIDBufferId = 0;
};

