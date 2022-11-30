#include "Batch.h"

#include <fmt/format.h>

#include "ki/uuid.h"

#include "model/Node.h"
#include "registry/NodeType.h"

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
    const glm::mat4& model,
    const glm::mat3& normal,
    int objectID) noexcept
{
    m_modelMatrices.push_back(model);

    if (m_useObjectIDBuffer) {
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

void Batch::prepare(
    const Assets& assets,
    int bufferSize) noexcept
{
    if (m_prepared) return;
    m_prepared = true;

    m_bufferSize = bufferSize;

    constexpr int bufferFlags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

    // model
    {
        m_modelMatrices.reserve(m_bufferSize);

        m_modelBuffer.create();
        m_modelBuffer.initEmpty(m_bufferSize * sizeof(glm::mat4), bufferFlags);
    }

    // normal
    {
        m_normalMatrices.reserve(m_bufferSize);

        m_normalBuffer.create();
        m_normalBuffer.initEmpty(m_bufferSize * sizeof(glm::mat3), bufferFlags);
    }

    // objectIDs
    {
        m_objectIDs.reserve(m_bufferSize);

        m_objectIDBuffer.create();
        m_objectIDBuffer.initEmpty(m_bufferSize * sizeof(glm::vec4), bufferFlags);
    }

    KI_DEBUG(fmt::format(
        "BATCHL: size={}, model={}, normal={}, objectID={}",
        m_bufferSize, m_modelBuffer, m_normalBuffer, m_objectIDBuffer));
}

void Batch::prepareMesh(GLVertexArray& vao)
{
    // model
    {
        KI_GL_CALL(glVertexArrayVertexBuffer(vao, VBO_MODEL_MATRIX_BINDING, m_modelBuffer, 0, sizeof(glm::mat4)));

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
        KI_GL_CALL(glVertexArrayVertexBuffer(vao, VBO_NORMAL_MATRIX_BINDING, m_normalBuffer, 0, sizeof(glm::mat3)));

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
        KI_GL_CALL(glVertexArrayVertexBuffer(vao, VBO_OBJECT_ID_BINDING, m_objectIDBuffer, 0, sizeof(glm::vec4)));

        glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_OBJECT_ID);

        glVertexArrayAttribFormat(vao, ATTR_INSTANCE_OBJECT_ID, 4, GL_FLOAT, GL_FALSE, 0);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.g-truc.net/post-0363.html
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_OBJECT_ID_BINDING, 1);

        glVertexArrayAttribBinding(vao, ATTR_INSTANCE_OBJECT_ID, VBO_OBJECT_ID_BINDING);
    }
}

void Batch::update(size_t count) noexcept
{
    if (count > m_bufferSize) {
        KI_WARN_SB("BATCH::CUT_OFF_BUFFER: count=" << count << " batchSize=" << m_bufferSize);
        count = m_bufferSize;
    }

    // TODO KI Map COHERRENT + PERSISTENT
    // => can be MUCH faster

    m_modelBuffer.update(0, count * sizeof(glm::mat4), m_modelMatrices.data());

    if (m_useObjectIDBuffer) {
        m_objectIDBuffer.update(0, count * sizeof(glm::vec4), m_objectIDs.data());
    }
    else {
        m_normalBuffer.update(0, count * sizeof(glm::mat3), m_normalMatrices.data());
    }
}

void Batch::bind(const RenderContext& ctx, Shader* shader) noexcept
{
    m_modelMatrices.clear();
    m_normalMatrices.clear();
    m_objectIDs.clear();
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

    if (type.m_flags.instanced) {
        node.bind(ctx, shader);
        node.draw(ctx);
        return;
    }

    node.bindBatch(ctx, *this);

    flushIfNeeded(ctx, type);
}

void Batch::drawAll(
    const RenderContext& ctx,
    NodeType& type,
    const std::vector<glm::mat4>& modelMatrices,
    const std::vector<glm::mat3>& normalMatrices,
    const std::vector<int>& objectIDs) noexcept
{
    for (int i = 0; i < modelMatrices.size(); i++) {
        add(modelMatrices[i], normalMatrices[i], objectIDs[i]);
        if (m_modelMatrices.size() >= m_bufferSize) {
            flush(ctx, type);
        }
    }
    flushIfNeeded(ctx, type);
}

void Batch::flushIfNeeded(const RenderContext& ctx, const NodeType& type) noexcept
{
    if (m_modelMatrices.size() < m_bufferSize) return;
    flush(ctx, type);
}

void Batch::flush(const RenderContext& ctx, const NodeType& type) noexcept
{
    if (!type.m_mesh) return;
    if (type.m_flags.root) return;
    if (type.m_flags.origo) return;

    int drawCount = m_modelMatrices.size();
    if (drawCount == 0) return;

    update(drawCount);

    if (ctx.assets.glDebug) {
        type.m_boundShader->validateProgram();
    }
    type.m_mesh->drawInstanced(ctx, drawCount);

    m_modelMatrices.clear();
    m_normalMatrices.clear();
    m_objectIDs.clear();
}
