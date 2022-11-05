#include "Batch.h"

#include <fmt/format.h>

#include "ki/uuid.h"

#include "model/Node.h"
#include "NodeType.h"

Batch::Batch()
{
}

Batch:: ~Batch()
{
    //    glDeleteBuffers(1, &modelBuffer);
    //    glDeleteBuffers(1, &normalBuffer);
    //    glDeleteBuffers(1, &objectIDBuffer);
}

void Batch::add(const glm::mat4& model, const glm::mat3& normal, int objectID) noexcept
{
    m_modelMatrices.push_back(model);

    if (objectIDBuffer) {
        int r = (objectID & 0x000000FF) >> 0;
        int g = (objectID & 0x0000FF00) >> 8;
        int b = (objectID & 0x00FF0000) >> 16;

        m_objectIDs.emplace_back(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    }
    else {
        m_normalMatrices.push_back(normal);
    }
}

void Batch::reserve(size_t count) noexcept
{
    m_modelMatrices.reserve(count);
    m_normalMatrices.reserve(count);
    m_objectIDs.reserve(count);
}

int Batch::size() noexcept
{
    return m_modelMatrices.size();
}

void Batch::clear() noexcept
{
    m_modelMatrices.clear();
    m_normalMatrices.clear();
    m_objectIDs.clear();
}

void Batch::prepare(NodeType& type) noexcept
{
    if (m_prepared) return;
    m_prepared = true;

    if (!type.m_mesh) return;

    if (staticBuffer) {
        batchSize = m_modelMatrices.size();
    }

    if (batchSize == 0) return;

    const int vao = type.m_mesh->m_buffers.VAO;
    constexpr int bufferFlags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

    // model
    {
        m_modelMatrices.reserve(batchSize);

        glCreateBuffers(1, &m_modelBufferId);
        glNamedBufferStorage(m_modelBufferId, batchSize * sizeof(glm::mat4), nullptr, bufferFlags);

        glVertexArrayVertexBuffer(vao, VBO_MODEL_MATRIX_BINDING, m_modelBufferId, 0, sizeof(glm::mat4));
        glBindBuffer(GL_ARRAY_BUFFER, m_modelBufferId);

        // NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
        constexpr GLsizei vecSize = sizeof(glm::vec4);

        for (int i = 0; i < 4; i++) {
            glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i);
            glVertexArrayAttribFormat(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i, 4, GL_FLOAT, GL_FALSE, i * vecSize);
            glVertexArrayAttribBinding(vao, ATTR_INSTANCE_MODEL_MATRIX_1 + i, VBO_MODEL_MATRIX_BINDING);
        }

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_MODEL_MATRIX_BINDING, 1);
    }

    // normal
    {
        m_normalMatrices.reserve(batchSize);

        glCreateBuffers(1, &m_normalBufferId);
        glNamedBufferStorage(m_normalBufferId, batchSize * sizeof(glm::mat3), nullptr, bufferFlags);

        glVertexArrayVertexBuffer(vao, VBO_NORMAL_MATRIX_BINDING, m_normalBufferId, 0, sizeof(glm::mat3));
        glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferId);

        // NOTE mat3 as vertex attributes *REQUIRES* hacky looking approach
        constexpr GLsizei vecSize = sizeof(glm::vec3);

        for (int i = 0; i < 3; i++) {
            glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i);
            glVertexArrayAttribFormat(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i, 3, GL_FLOAT, GL_FALSE, i * vecSize);
            glVertexArrayAttribBinding(vao, ATTR_INSTANCE_NORMAL_MATRIX_1 + i, VBO_NORMAL_MATRIX_BINDING);
        }

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_NORMAL_MATRIX_BINDING, 1);
    }

    // objectIDs
    {
        m_objectIDs.reserve(batchSize);

        glCreateBuffers(1, &m_objectIDBufferId);
        glNamedBufferStorage(m_objectIDBufferId, batchSize * sizeof(glm::vec4), nullptr, bufferFlags);

        glVertexArrayVertexBuffer(vao, VBO_OBJECT_ID_BINDING, m_objectIDBufferId, 0, sizeof(glm::vec4));
        glBindBuffer(GL_ARRAY_BUFFER, m_objectIDBufferId);

        glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_OBJECT_ID);

        glVertexArrayAttribFormat(vao, ATTR_INSTANCE_OBJECT_ID, 4, GL_FLOAT, GL_FALSE, 0);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.g-truc.net/post-0363.html
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_OBJECT_ID_BINDING, 1);

        glVertexArrayAttribBinding(vao, ATTR_INSTANCE_OBJECT_ID, VBO_OBJECT_ID_BINDING);
    }

    if (staticBuffer) {
        update(batchSize);
    }

    KI_DEBUG(fmt::format(
        "BATCHL: {} - model={}, normal={}, objectID={}",
        type.str(), m_modelBufferId, m_normalBufferId, m_objectIDBufferId));
}

void Batch::update(size_t count) noexcept
{
    if (batchSize == 0) return;

    if (count > batchSize) {
        KI_WARN_SB("BATCH::CUT_OFF_BUFFER: count=" << count << " batchSize=" << batchSize);
        count = batchSize;
    }

    // TODO KI Map COHERRENT + PERSISTENT
    // => can be MUCH faster

    glNamedBufferSubData(m_modelBufferId, 0, count * sizeof(glm::mat4), &m_modelMatrices[0]);

    if (objectIDBuffer) {
        glNamedBufferSubData(m_objectIDBufferId, 0, count * sizeof(glm::vec4), &m_objectIDs[0]);
    }
    else {
        glNamedBufferSubData(m_normalBufferId, 0, count * sizeof(glm::mat3), &m_normalMatrices[0]);
    }

    //KI_GL_UNBIND(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Batch::bind(const RenderContext& ctx, Shader* shader) noexcept
{
    if (batchSize == 0) return;

    if (!staticBuffer) {
        m_modelMatrices.clear();
        m_normalMatrices.clear();
        m_objectIDs.clear();
    }
}

void Batch::draw(
    const RenderContext& ctx,
    Node& node,
    Shader* shader) noexcept
{
    const auto& type = *node.m_type.get();

    if (!type.m_mesh) return;
    if (type.m_flags.root) return;
    if (type.m_flags.origo) return;

    const auto& volume = node.getVolume();
    if (ctx.useFrustum &&
        ctx.assets.frustumEnabled &&
        !type.m_flags.noFrustum &&
        volume &&
        !volume->isOnFrustum(ctx.frustum, node.getMatrixLevel(), node.getWorldModelMatrix()))
    {
        ctx.skipCount += 1;
        return;
    }

    ctx.drawCount += 1;

    if (type.m_flags.instanced) {
        node.bind(ctx, shader);
        node.draw(ctx);
        return;
    }
    if (batchSize == 0) return;

    node.bindBatch(ctx, *this);

    if (m_modelMatrices.size() < batchSize) return;

    flush(ctx, type);
}

void Batch::flush(const RenderContext& ctx, const NodeType& type) noexcept
{
    if (!type.m_mesh) return;
    if (type.m_flags.root) return;
    if (type.m_flags.origo) return;

    if (batchSize == 0) return;

    int drawCount;
    if (staticBuffer) {
        drawCount = staticDrawCount;
    }
    else {
        drawCount = m_modelMatrices.size();
    }

    if (drawCount == 0) return;

    if (!staticBuffer) {
        update(drawCount);
    }

    if (ctx.assets.glDebug) {
        type.m_boundShader->validateProgram();
    }
    type.m_mesh->drawInstanced(ctx, drawCount);

    if (!staticBuffer) {
        m_modelMatrices.clear();
        m_normalMatrices.clear();
        m_objectIDs.clear();
    }
}
