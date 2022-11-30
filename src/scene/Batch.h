#pragma once

#include "asset/Shader.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class RenderContext;
class NodeType;
class Node;

// NOTE KI use single shared UBO buffer for rendering
// => less resources needed
//
// https://stackoverflow.com/questions/15438605/can-a-vbo-be-bound-to-multiple-vaos
// https://www.khronos.org/opengl/wiki/Vertex_Specification#Index_buffers
//
class Batch final
{
public:
    Batch();
    ~Batch() = default;

    // https://stackoverflow.com/questions/7823845/disable-compiler-generated-copy-assignment-operator
    Batch(const Batch&) = delete;
    Batch& operator=(const Batch&) = delete;

    void add(
        const glm::mat4& model,
        const glm::mat3& normal,
        int objectID) noexcept;

    void reserve(size_t count) noexcept;
    int size() noexcept;

    void clear() noexcept;

    void prepare(
        const Assets& assets,
        int bufferSize) noexcept;

    void prepareMesh(GLVertexArray& vao);

    void update(size_t count) noexcept;
    void bind(const RenderContext& ctx, Shader* shader) noexcept;
    void draw(const RenderContext& ctx, Node& node, Shader* shader) noexcept;

    void drawAll(
        const RenderContext& ctx,
        NodeType& type,
        const std::vector<glm::mat4>& m_modelMatrices,
        const std::vector<glm::mat3>& m_normalMatrices,
        const std::vector<int>& m_objectIDs) noexcept;

    void flushIfNeeded(const RenderContext& ctx, const NodeType& type) noexcept;
    void flush(const RenderContext& ctx, const NodeType& type) noexcept;

public:
    const int m_id;

    bool m_dirty = false;
    bool m_useObjectIDBuffer = false;

private:
    bool m_prepared = false;

    int m_bufferSize = -1;

    std::vector<glm::mat4> m_modelMatrices;
    std::vector<glm::mat3> m_normalMatrices;
    std::vector<glm::vec4> m_objectIDs;

    GLBuffer m_modelBuffer;
    GLBuffer m_normalBuffer;
    GLBuffer m_objectIDBuffer;
};

