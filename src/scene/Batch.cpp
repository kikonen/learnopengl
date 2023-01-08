#include "Batch.h"

#include <mutex>
#include <fmt/format.h>

#include "glm/glm.hpp"

#include "ki/uuid.h"

#include "asset/VertexEntry.h"

#include "model/Node.h"
#include "registry/MeshType.h"

#include "scene/RenderContext.h"


namespace {
    int idBase = 0;

    std::mutex type_id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }
}

Batch::Batch()
    : m_id(nextID())
{
}

void Batch::add(
    const RenderContext& ctx,
    const int entityIndex)
{
    if (entityIndex < 0) throw std::runtime_error{ "INVALID_ENTITY_INDEX" };

    auto& entry = m_entries.emplace_back();
    entry.m_entityIndex = entityIndex;

    auto& top = m_batches.back();
    top.m_drawCount += 1;

    flushIfNeeded(ctx);
}

void Batch::addAll(
    const RenderContext& ctx,
    const std::vector<int> entityIndeces
)
{
    BatchCommand save = m_batches.back();
    save.m_drawCount = 0;

    for (const auto& entityIndex : entityIndeces) {
        add(ctx, entityIndex);
    }
}

void Batch::reserve(size_t count) noexcept
{
    m_entries.reserve(count);
}

size_t Batch::size() noexcept
{
    return m_entries.size();
}

void Batch::bind() noexcept
{
    clear();
    m_draw.bind();
}

void Batch::clear() noexcept
{
    m_batches.clear();
    m_entries.clear();
}

void Batch::prepare(
    const Assets& assets,
    int entryCount) noexcept
{
    if (m_prepared) return;
    m_prepared = true;

    m_entryCount = entryCount;

    {
        m_offset = 0;

        constexpr int sz = sizeof(BatchEntry);
        //constexpr int bufferFlags = GL_DYNAMIC_STORAGE_BIT;// | GL_MAP_WRITE_BIT;

        m_vbo.createEmpty(m_entryCount * sz, GL_DYNAMIC_STORAGE_BIT);
    }

    m_draw.prepare(20, 100);

    m_entries.reserve(m_entryCount);

    KI_DEBUG(fmt::format(
        "BATCHL: size={}, buffer={}",
        m_entryCount, m_vbo));
}

void Batch::prepareVAO(
    GLVertexArray& vao,
    bool singleMaterial)
{
    glVertexArrayVertexBuffer(vao, VBO_BATCH_BINDING, m_vbo, m_offset, sizeof(BatchEntry));

    //// model
    //{
    //    // NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
    //    constexpr GLsizei vecSize = sizeof(glm::vec4);

    //    for (int i = 0; i < 4; i++) {
    //        glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i);
    //        glVertexArrayAttribFormat(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i, 4, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, m_modelMatrix) + i * vecSize);
    //        glVertexArrayAttribBinding(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i, VBO_BATCH_BINDING);
    //    }
    //}

    //// normal
    //{
    //    // NOTE mat3 as vertex attributes *REQUIRES* hacky looking approach
    //    constexpr GLsizei vecSize = sizeof(glm::vec3);

    //    for (int i = 0; i < 3; i++) {
    //        glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i);
    //        glVertexArrayAttribFormat(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i, 3, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, m_normalMatrix) + i * vecSize);
    //        glVertexArrayAttribBinding(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i, VBO_BATCH_BINDING);
    //    }
    //}

    //// objectIDs
    //{
    //    glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_OBJECT_ID);
    //    glVertexArrayAttribFormat(vao, ATTR_INSTANCE_OBJECT_ID, 4, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, m_objectID));
    //    glVertexArrayAttribBinding(vao, ATTR_INSTANCE_OBJECT_ID, VBO_BATCH_BINDING);
    //}

    //// material
    //{
    //    glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_MATERIAL_INDEX);
    //    glVertexArrayAttribFormat(vao, ATTR_INSTANCE_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, m_materialIndex));
    //    glVertexArrayAttribBinding(vao, ATTR_INSTANCE_MATERIAL_INDEX, VBO_BATCH_BINDING);
    //}

    //// highlight
    //{
    //    glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_HIGHLIGHT_INDEX);
    //    glVertexArrayAttribFormat(vao, ATTR_INSTANCE_HIGHLIGHT_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, m_highlightIndex));
    //    glVertexArrayAttribBinding(vao, ATTR_INSTANCE_HIGHLIGHT_INDEX, VBO_BATCH_BINDING);
    //}

    // entityIndex
    {
        glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_ENTITY_INDEX);
        glVertexArrayAttribFormat(vao, ATTR_INSTANCE_ENTITY_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, m_entityIndex));
        glVertexArrayAttribBinding(vao, ATTR_INSTANCE_ENTITY_INDEX, VBO_BATCH_BINDING);
    }

    {
        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.g-truc.net/post-0363.html
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_BATCH_BINDING, 1);
    }
}

void Batch::update() noexcept
{
    m_vbo.update(m_offset, m_entries.size() * sizeof(BatchEntry), m_entries.data());
}

void Batch::addCommand(
    const RenderContext& ctx,
    MeshType* type,
    Shader* shader)
{
    auto& cmd = m_batches.emplace_back();

    cmd.m_vao = type->m_vao;
    cmd.m_shader = shader;
    cmd.m_drawOptions = &type->m_drawOptions;
}

void Batch::draw(
    const RenderContext& ctx,
    Node& node,
    Shader* shader)
{
    const auto type = node.m_type;

    if (!type->getMesh()) return;
    if (type->m_flags.noRender) return;
    if (type->m_flags.noDisplay) return;

    auto& obb = node.getOBB();
    //const auto mvp = ctx.m_matrices.projected * node.getModelMatrix();

    const auto& volume = node.getVolume();
    if (ctx.m_useFrustum &&
        ctx.assets.frustumEnabled &&
        !type->m_flags.noFrustum &&
        volume &&
        !volume->isOnFrustum(
            ctx.m_camera.getFrustum(),
            node.getMatrixLevel(),
            node.getWorldModelMatrix()))
    {
        //!obb.inFrustum(
        //    ctx.m_camera.getProjectedLevel(),
        //    ctx.m_camera.getProjected(),
        //    node.getMatrixLevel(),
        //    node.getWorldModelMatrix()))

            //volume &&
            //!volume->isOnFrustum(
            //    *ctx.getFrustum(),
            //    node.getMatrixLevel(),
            //    node.getWorldModelMatrix()))
            //!volume->isOnFrustum(*ctx.getFrustum(), node.getMatrixLevel(), node.getWorldModelMatrix()))
        ctx.m_skipCount += 1;
        return;
    }

    ctx.m_drawCount += 1;

    {
        const bool useBlend = ctx.m_useBlend;
        bool change = true;
        if (!m_batches.empty()) {
            auto& top = m_batches.back();
            change = shader != top.m_shader ||
                type->m_vao != top.m_vao ||
                !top.m_drawOptions->isSameDrawCommand(type->m_drawOptions, useBlend);
        }

        if (change) {
            addCommand(ctx, type, shader);
        }

        auto& top = m_batches.back();
        top.m_materialVBO = &type->m_materialVBO;
    }

    node.bindBatch(ctx, *this);
    flushIfNeeded(ctx);
}

void Batch::flushIfNeeded(
    const RenderContext& ctx)
{
    if (m_entries.size() < m_entryCount) return;
    flush(ctx, false);
}

void Batch::flush(
    const RenderContext& ctx,
    bool release)
{
    if (m_entries.empty()) {
        m_batches.clear();
        return;
    }

    update();
    drawInstanced(ctx);

    m_batches.clear();
    m_entries.clear();
}

void Batch::drawInstanced(
    const RenderContext& ctx)
{
    const bool useBlend = ctx.m_useBlend;
    const Shader* boundShader{ nullptr };
    const GLVertexArray* boundVAO{ nullptr };
    const backend::DrawOptions* boundDrawOptions{ nullptr };
    int  baseInstance = 0;

    backend::DrawIndirectCommand indirect;

    for (auto& curr : m_batches) {
        if (ctx.assets.glDebug) {
            curr.m_shader->validateProgram();
        }

        bool sameDraw = boundShader == curr.m_shader &&
            boundVAO == curr.m_vao &&
            boundDrawOptions &&
            boundDrawOptions->isSameMultiDraw(*curr.m_drawOptions, useBlend);

        if (!sameDraw) {
            if (boundShader) {
                m_draw.flush(ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
            }

            boundShader = curr.m_shader;
            boundVAO = curr.m_vao;
            boundDrawOptions = curr.m_drawOptions;
        }

        // NOTE KI baseVertex usage
        // https://community.khronos.org/t/vertex-buffer-management-with-indirect-drawing/77272
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Instanced_arrays
        //
        const auto& drawOptions = *curr.m_drawOptions;

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            backend::DrawElementsIndirectCommand& cmd = indirect.element;

            cmd.count = drawOptions.indexCount;
            cmd.instanceCount = curr.m_drawCount;
            cmd.firstIndex = drawOptions.indexOffset / sizeof(GLuint);
            cmd.baseVertex = drawOptions.vertexOffset / sizeof(VertexEntry);
            cmd.baseInstance = baseInstance;

            m_draw.send(indirect, ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            backend::DrawArraysIndirectCommand& cmd = indirect.array;

            cmd.vertexCount = drawOptions.indexCount;
            cmd.instanceCount = curr.m_drawCount;
            cmd.firstVertex = drawOptions.indexOffset / sizeof(GLuint);
            cmd.baseInstance = baseInstance;

            m_draw.send(indirect, ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
        }
        else {
            // NOTE KI "none" no drawing
            KI_INFO("no render");
        }

        baseInstance += curr.m_drawCount;
    }

    if (boundShader) {
        m_draw.flush(ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
    }
}

