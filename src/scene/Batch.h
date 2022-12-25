#pragma once

#include "asset/Shader.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "backend/DrawElementsIndirectCommand.h"
#include "backend/DrawIndirectCommand.h"
#include "backend/DrawOptions.h"

#include "BatchCommand.h"
#include "BatchEntry.h"


class RenderContext;
class MeshType;
class Node;
class MaterialVBO;

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
        int objectID,
        bool selected) noexcept;

    void addAll(
        const RenderContext& ctx,
        const std::vector<glm::mat4>& modelMatrices,
        const std::vector<glm::mat3>& normalMatrices,
        const std::vector<int>& objectIDs,
        bool selected);

    void reserve(size_t count) noexcept;
    int size() noexcept;

    void clear() noexcept;

    void prepare(
        const Assets& assets,
        int bufferSize) noexcept;

    void prepareVAO(GLVertexArray& vao, bool singleMaterial);

    void draw(
        const RenderContext& ctx,
        Node& node,
        Shader* shader);

    void flush(
        const RenderContext& ctx,
        bool release = true);

private:
    void update(size_t count) noexcept;
    void updateCommands() noexcept;

    void bind(
        const RenderContext& ctx,
        MeshType* type,
        Shader* shader);

    void clear();

    void drawInstanced(
        const RenderContext& ctx,
        int drawCount);

    void drawPending(
        const RenderContext& ctx,
        GLVertexArray* vao,
        backend::DrawOptions drawOptions);

    void flushIfNeeded(const RenderContext& ctx);

public:
    const int m_id;

    bool m_dirty = false;
    bool m_useObjectIDBuffer = false;
    bool m_selection = false;

private:
    bool m_prepared = false;

    int m_bufferSize = -1;

    RenderContext* m_currentRenderContext{ nullptr };
    std::vector<BatchCommand> m_batches;

    std::vector<BatchEntry> m_entries;

    std::vector<backend::DrawIndirectCommand> m_drawCommands;

    GLBuffer m_drawBuffer;

    GLBuffer m_buffer;

    GLBuffer m_materialBuffer;

    int m_offset = 0;
};
