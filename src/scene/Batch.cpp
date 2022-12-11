#include "Batch.h"

#include <fmt/format.h>

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

    {
        const int modelSize = m_bufferSize * sizeof(glm::mat4);
        const int normalSize = m_bufferSize * sizeof(glm::mat3);
        const int objectSize = m_bufferSize * sizeof(glm::vec4);

        const int sz = modelSize + normalSize + objectSize;

        m_modelOffset = 0;
        m_normalOffset = modelSize;
        m_objectOffset = modelSize + normalSize;

        constexpr int bufferFlags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;

        m_buffer.create();
        m_buffer.initEmpty(sz, bufferFlags);
    }

    // model
    {
        m_modelMatrices.reserve(m_bufferSize);
    }

    // normal
    {
        m_normalMatrices.reserve(m_bufferSize);
    }

    // objectIDs
    {
        m_objectIDs.reserve(m_bufferSize);
    }

    KI_DEBUG(fmt::format(
        "BATCHL: size={}, buffer={}",
        m_bufferSize, m_buffer));
}

void Batch::prepareMesh(GLVertexArray& vao)
{
    // model
    {
        glVertexArrayVertexBuffer(vao, VBO_MODEL_MATRIX_BINDING, m_buffer, m_modelOffset, sizeof(glm::mat4));

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
        glVertexArrayVertexBuffer(vao, VBO_NORMAL_MATRIX_BINDING, m_buffer, m_normalOffset, sizeof(glm::mat3));

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
        glVertexArrayVertexBuffer(vao, VBO_OBJECT_ID_BINDING, m_buffer, m_objectOffset, sizeof(glm::vec4));

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

    m_buffer.update(m_modelOffset, count * sizeof(glm::mat4), m_modelMatrices.data());

    if (m_useObjectIDBuffer) {
        m_buffer.update(m_objectOffset, count * sizeof(glm::vec4), m_objectIDs.data());
    }
    else {
        m_buffer.update(m_normalOffset, count * sizeof(glm::mat3), m_normalMatrices.data());
    }
}

void Batch::bind(const RenderContext& ctx, Shader* shader) noexcept
{
    m_boundShader = shader;

    m_modelMatrices.clear();
    m_normalMatrices.clear();
    m_objectIDs.clear();
}

void Batch::draw(
    const RenderContext& ctx,
    Node& node,
    Shader* shader) noexcept
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
    MeshType& type,
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

void Batch::flushIfNeeded(const RenderContext& ctx, const MeshType& type) noexcept
{
    if (m_modelMatrices.size() < m_bufferSize) return;
    flush(ctx, type);
}

void Batch::flush(const RenderContext& ctx, const MeshType& type) noexcept
{
    const auto& mesh = type.getMesh();

    if (!mesh) return;
    if (type.m_flags.root) return;
    if (type.m_flags.origo) return;

    int drawCount = m_modelMatrices.size();
    if (drawCount == 0) return;

    update(drawCount);

    if (ctx.assets.glDebug) {
        m_boundShader->validateProgram();
    }
    mesh->drawInstanced(ctx, drawCount);

    m_modelMatrices.clear();
    m_normalMatrices.clear();
    m_objectIDs.clear();
}
