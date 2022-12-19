#include "Batch.h"

#include <fmt/format.h>

#include "glm/glm.hpp"

#include "ki/uuid.h"

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

    const glm::mat4 BASE_MAT_1 = glm::mat4(1.0f);
    const glm::vec3 SCALE{ 1.02f };
    const glm::mat4 SELECTION_MAT = glm::scale(BASE_MAT_1, SCALE);
}

Batch::Batch()
    : m_id(nextID())
{
}

void Batch::add(
    const RenderContext& ctx,
    const glm::mat4& model,
    const glm::mat3& normal,
    int objectID,
    bool selected) noexcept
{
    BatchEntry entry;

    if (selected && m_selection) {
        entry.modelMatrix = model * SELECTION_MAT;
        entry.normalMatrix = normal;
    } else {
        entry.modelMatrix = model;
        entry.normalMatrix = normal;
    }

    if (m_useObjectIDBuffer) {
        int r = (objectID & 0x000000FF) >> 0;
        int g = (objectID & 0x0000FF00) >> 8;
        int b = (objectID & 0x00FF0000) >> 16;

        entry.objectID.r = r / 255.0f;
        entry.objectID.g = g / 255.0f;
        entry.objectID.b = b / 255.0f;
        entry.objectID.a = 1.0f;
    }

    m_entries.push_back(entry);

    flushIfNeeded(ctx);
}

void Batch::addAll(
    const RenderContext& ctx,
    const std::vector<glm::mat4>& modelMatrices,
    const std::vector<glm::mat3>& normalMatrices,
    const std::vector<int>& objectIDs,
    bool selected)
{
    for (int i = 0; i < modelMatrices.size(); i++) {
        add(ctx, modelMatrices[i], normalMatrices[i], objectIDs[i], selected);
    }
}

void Batch::reserve(size_t count) noexcept
{
    m_entries.reserve(count);
}

int Batch::size() noexcept
{
    return m_entries.size();
}

void Batch::clear() noexcept
{
    m_entries.clear();
}

void Batch::prepare(
    const Assets& assets,
    int bufferSize) noexcept
{
    if (m_prepared) return;
    m_prepared = true;

    m_bufferSize = bufferSize;

    {
        m_offset = 0;

        constexpr int sz = sizeof(BatchEntry);
        //constexpr int bufferFlags = GL_DYNAMIC_STORAGE_BIT;// | GL_MAP_WRITE_BIT;

        m_buffer.create();
        m_buffer.initEmpty(m_bufferSize * sz, GL_DYNAMIC_STORAGE_BIT);
    }

    m_entries.reserve(m_bufferSize);

    KI_DEBUG(fmt::format(
        "BATCHL: size={}, buffer={}",
        m_bufferSize, m_buffer));
}

void Batch::prepareMesh(GLVertexArray& vao)
{
    glVertexArrayVertexBuffer(vao, VBO_BATCH_BINDING, m_buffer, m_offset, sizeof(BatchEntry));

    // model
    {
        // NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
        constexpr GLsizei vecSize = sizeof(glm::vec4);

        for (int i = 0; i < 4; i++) {
            glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i);
            glVertexArrayAttribFormat(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i, 4, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, modelMatrix) + i * vecSize);
            glVertexArrayAttribBinding(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i, VBO_BATCH_BINDING);
        }
    }

    // normal
    {
        // NOTE mat3 as vertex attributes *REQUIRES* hacky looking approach
        constexpr GLsizei vecSize = sizeof(glm::vec3);

        for (int i = 0; i < 3; i++) {
            glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i);
            glVertexArrayAttribFormat(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i, 3, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, normalMatrix) + i * vecSize);
            glVertexArrayAttribBinding(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i, VBO_BATCH_BINDING);
        }
    }

    // objectIDs
    {
        glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_OBJECT_ID);
        glVertexArrayAttribFormat(vao, ATTR_INSTANCE_OBJECT_ID, 4, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, objectID));
        glVertexArrayAttribBinding(vao, ATTR_INSTANCE_OBJECT_ID, VBO_BATCH_BINDING);

    }

    {
        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.g-truc.net/post-0363.html
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_BATCH_BINDING, 1);
    }

    KI_GL_CHECK("2");
}

void Batch::update(size_t count) noexcept
{
    if (count > m_bufferSize) {
        KI_WARN_SB("BATCH::CUT_OFF_BUFFER: count=" << count << " batchSize=" << m_bufferSize);
        count = m_bufferSize;
    }

    // TODO KI Map COHERRENT + PERSISTENT
    // => can be MUCH faster
    m_buffer.update(m_offset, count * sizeof(BatchEntry), m_entries.data());
}

void Batch::bind(
    const RenderContext& ctx,
    MeshType* type,
    Shader* shader)
{
    if (m_boundType) return;

    m_boundType = type;
    m_boundShader = shader;

    m_entries.clear();
}

void Batch::unbind()
{
    m_boundType = nullptr;
    m_boundShader = nullptr;

    m_entries.clear();
}

void Batch::draw(
    const RenderContext& ctx,
    Node& node,
    Shader* shader)
{
    const auto& type = *node.m_type;

    if (!type.getMesh()) return;
    if (type.m_flags.root) return;
    if (type.m_flags.origo) return;

    auto& obb = node.getOBB();
    //const auto mvp = ctx.m_matrices.projected * node.getModelMatrix();

    const auto& volume = node.getVolume();
    if (ctx.m_useFrustum &&
        ctx.assets.frustumEnabled &&
        !type.m_flags.noFrustum &&
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

    if (node.m_type != m_boundType) {
        flush(ctx);
        unbind();
    }

    bind(ctx, node.m_type, shader);
    node.bindBatch(ctx, *this);
    flushIfNeeded(ctx);
}

void Batch::flushIfNeeded(
    const RenderContext& ctx)
{
    if (m_entries.size() < m_bufferSize) return;
    flush(ctx, false);
}

void Batch::flush(
    const RenderContext& ctx,
    bool release)
{
    if (!m_boundType) return;

    int drawCount = m_entries.size();

    if (drawCount == 0) {
        if (release) {
            unbind();
        }
        return;
    }

    update(drawCount);

    if (ctx.assets.glDebug) {
        m_boundShader->validateProgram();
    }
    {
        m_boundShader->bind(ctx.state);

        auto& type = *m_boundType;
        const auto& mesh = type.getMesh();
        const auto& drawOptions = type.m_drawOptions;

        ctx.bindDraw(drawOptions.renderBack, drawOptions.wireframe);
        ctx.state.useVAO(type.m_vao);

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            glDrawElementsInstanced(
                drawOptions.mode,
                drawOptions.indexCount,
                GL_UNSIGNED_INT,
                (void*)drawOptions.indexOffset,
                drawCount);
            //int baseInstance = 0;

            //glDrawElementsInstancedBaseVertexBaseInstance(
            //    GL_TRIANGLES,
            //    m_triCount * 3,
            //    GL_UNSIGNED_INT,
            //    (void*)m_vertexVBO.m_indexOffset,
            //    instanceCount,
            //    m_vertexVBO.m_baseVertex,
            //    baseInstance);
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            glDrawArraysInstanced(
                drawOptions.mode,
                drawOptions.indexFirst,
                drawOptions.indexCount,
                drawCount);

            //int baseInstance = 0;

            //glDrawArraysInstancedBaseInstance(
            //    GL_TRIANGLE_STRIP,
            //    0,
            //    4,
            //    instanceCount,
            //    baseInstance);
        }
        else {
            // NOTE KI "none" no drawing
            KI_INFO("no render");
        }
    }

    m_entries.clear();

    if (release) {
        unbind();
    }
}
