#pragma once


#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"
#include "kigl/GLBufferRange.h"

#include "backend/DrawOptions.h"
#include "backend/DrawBuffer.h"

#include "BatchCommand.h"
#include "BatchEntry.h"

class Shader;
class RenderContext;
class MeshType;
class Node;
class MaterialVBO;
class ShaderRegistry;


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

    void addInstanced(
        const RenderContext& ctx,
        int firstIndex,
        int count);

    void bind() noexcept;

    void prepare(
        const Assets& assets,
        ShaderRegistry& shaders,
        int entryCount = -1,
        int bufferCount = -1);

    void prepareVAO(GLVertexArray& vao, bool singleMaterial);

    void draw(
        const RenderContext& ctx,
        Node& node,
        Shader* shader);

    void flush(
        const RenderContext& ctx);

private:
    void addCommand(
        const RenderContext& ctx,
        MeshType* type,
        Shader* shader);

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

    std::vector<int> m_entityIndeces;

    std::unique_ptr<backend::DrawBuffer> m_draw;
};
