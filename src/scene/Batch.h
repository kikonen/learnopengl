#pragma once

#include "asset/Shader.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"
#include "kigl/GLBufferRange.h"

#include "backend/DrawElementsIndirectCommand.h"
#include "backend/DrawIndirectCommand.h"
#include "backend/DrawOptions.h"
#include "backend/DrawBuffer.h"

#include "BatchCommand.h"
#include "BatchEntry.h"

class RenderContext;
class MeshType;
class Node;
class MaterialVBO;

constexpr int RANGE_COUNT = 3;

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
        const int entityIndex);

    void addAll(
        const RenderContext& ctx,
        const std::vector<int> entityIndeces);

    void reserve(size_t count) noexcept;
    size_t size() noexcept;

    void bind() noexcept;
    void clear() noexcept;

    void prepare(
        const Assets& assets,
        int entryCount) noexcept;

    void prepareVAO(GLVertexArray& vao, bool singleMaterial);

    void draw(
        const RenderContext& ctx,
        Node& node,
        Shader* shader);

    void flush(
        const RenderContext& ctx,
        bool release = true);

private:
    void update() noexcept;

    void addCommand(
        const RenderContext& ctx,
        MeshType* type,
        Shader* shader);

    void drawInstanced(
        const RenderContext& ctx);

    void flushIfNeeded(const RenderContext& ctx);

public:
    const int m_id;

    bool m_dirty = false;
    bool m_useObjectIDBuffer = false;
    bool m_highlight = false;

private:
    bool m_prepared = false;

    int m_entryCount = 0;

    RenderContext* m_currentRenderContext{ nullptr };
    std::vector<BatchCommand> m_batches;

    std::vector<BatchEntry> m_entries;

    GLBuffer m_vbo{ "batchVBO" };
    //BatchEntry* m_mapped;
    //int m_size = 0;

    //int m_range = 0;
    //GLBufferRange m_ranges[RANGE_COUNT];

    backend::DrawBuffer m_draw;

    int m_offset = 0;
};
