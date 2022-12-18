#pragma once

#include "asset/Shader.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class RenderContext;
class MeshType;
class Node;

#pragma pack(push, 1)
struct BatchEntry {
    glm::mat4 modelMatrix;
    glm::mat3 normalMatrix;
    glm::vec4 objectID;
};
#pragma pack(pop)

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
        const RenderContext& ctx,
        const glm::mat4& model,
        const glm::mat3& normal,
        int objectID) noexcept;

    void addAll(
        const RenderContext& ctx,
        const std::vector<glm::mat4>& modelMatrices,
        const std::vector<glm::mat3>& normalMatrices,
        const std::vector<int>& objectIDs);

    void reserve(size_t count) noexcept;
    int size() noexcept;

    void clear() noexcept;

    void prepare(
        const Assets& assets,
        int bufferSize) noexcept;

    void prepareMesh(GLVertexArray& vao);

    void draw(
        const RenderContext& ctx,
        Node& node,
        Shader* shader);

    void flush(
        const RenderContext& ctx,
        bool release = true);

private:
    void update(size_t count) noexcept;

    void bind(
        const RenderContext& ctx,
        MeshType* type,
        Shader* shader);
    void unbind();

    void flushIfNeeded(const RenderContext& ctx);

public:
    const int m_id;

    bool m_dirty = false;
    bool m_useObjectIDBuffer = false;

private:
    bool m_prepared = false;

    int m_bufferSize = -1;

    Shader* m_boundShader{ nullptr };
    MeshType* m_boundType{ nullptr };

    std::vector<BatchEntry> m_entries;

    GLBuffer m_buffer;

    int m_offset = 0;
};

