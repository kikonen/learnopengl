#include "Batch.h"

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

void Batch::add(const glm::mat4& model, const glm::mat3& normal, int objectID)
{
    m_modelMatrices.push_back(model);

    if (objectId) {
        int r = (objectID & 0x000000FF) >> 0;
        int g = (objectID & 0x0000FF00) >> 8;
        int b = (objectID & 0x00FF0000) >> 16;

        m_objectIDs.emplace_back(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    } else {
        m_normalMatrices.push_back(normal);
    }
}

void Batch::reserve(size_t count)
{
    m_modelMatrices.reserve(count);
    m_normalMatrices.reserve(count);
    m_objectIDs.reserve(count);
}

int Batch::size()
{
    return m_modelMatrices.size();
}

void Batch::clear()
{
    m_modelMatrices.clear();
    m_normalMatrices.clear();
    m_objectIDs.clear();
}

void Batch::prepare(NodeType* type)
{
    if (m_prepared) return;
    m_prepared = true;

    if (!type->mesh) return;
    if (batchSize == 0) return;

    KI_GL_CALL(glBindVertexArray(type->mesh->m_buffers.VAO));

    preparedBufferSize = batchSize;

    // model
    {
        m_modelMatrices.reserve(preparedBufferSize);

        glCreateBuffers(1, &m_modelBufferId);
        glNamedBufferStorage(m_modelBufferId, preparedBufferSize * sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);

        // NOTE mat4 as vertex attributes *REQUIRES* hacky looking approach
        constexpr GLsizei vecSize = sizeof(glm::vec4);

        glBindBuffer(GL_ARRAY_BUFFER, m_modelBufferId);

        glVertexAttribPointer(ATTR_INSTANCE_MODEL_MATRIX_1, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)0);
        glVertexAttribPointer(ATTR_INSTANCE_MODEL_MATRIX_2, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(1 * vecSize));
        glVertexAttribPointer(ATTR_INSTANCE_MODEL_MATRIX_3, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(2 * vecSize));
        glVertexAttribPointer(ATTR_INSTANCE_MODEL_MATRIX_4, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(3 * vecSize));

        glVertexAttribDivisor(ATTR_INSTANCE_MODEL_MATRIX_1, 1);
        glVertexAttribDivisor(ATTR_INSTANCE_MODEL_MATRIX_2, 1);
        glVertexAttribDivisor(ATTR_INSTANCE_MODEL_MATRIX_3, 1);
        glVertexAttribDivisor(ATTR_INSTANCE_MODEL_MATRIX_4, 1);

        glEnableVertexAttribArray(ATTR_INSTANCE_MODEL_MATRIX_1);
        glEnableVertexAttribArray(ATTR_INSTANCE_MODEL_MATRIX_2);
        glEnableVertexAttribArray(ATTR_INSTANCE_MODEL_MATRIX_3);
        glEnableVertexAttribArray(ATTR_INSTANCE_MODEL_MATRIX_4);
    }

    // normal
    {
        m_normalMatrices.reserve(preparedBufferSize);

        glCreateBuffers(1, &m_normalBufferId);
        glNamedBufferStorage(m_normalBufferId, preparedBufferSize * sizeof(glm::mat3), nullptr, GL_DYNAMIC_STORAGE_BIT);

        // NOTE mat3 as vertex attributes *REQUIRES* hacky looking approach
        constexpr GLsizei vecSize = sizeof(glm::vec3);

        glBindBuffer(GL_ARRAY_BUFFER, m_normalBufferId);

        glVertexAttribPointer(ATTR_INSTANCE_NORMAL_MATRIX_1, 3, GL_FLOAT, GL_FALSE, 3 * vecSize, (void*)0);
        glVertexAttribPointer(ATTR_INSTANCE_NORMAL_MATRIX_2, 3, GL_FLOAT, GL_FALSE, 3 * vecSize, (void*)(1 * vecSize));
        glVertexAttribPointer(ATTR_INSTANCE_NORMAL_MATRIX_3, 3, GL_FLOAT, GL_FALSE, 3 * vecSize, (void*)(2 * vecSize));

        glVertexAttribDivisor(ATTR_INSTANCE_NORMAL_MATRIX_1, 1);
        glVertexAttribDivisor(ATTR_INSTANCE_NORMAL_MATRIX_2, 1);
        glVertexAttribDivisor(ATTR_INSTANCE_NORMAL_MATRIX_3, 1);

        glEnableVertexAttribArray(ATTR_INSTANCE_NORMAL_MATRIX_1);
        glEnableVertexAttribArray(ATTR_INSTANCE_NORMAL_MATRIX_2);
        glEnableVertexAttribArray(ATTR_INSTANCE_NORMAL_MATRIX_3);
    }

    // objectIDs
    {
        m_objectIDs.reserve(preparedBufferSize);

        glCreateBuffers(1, &m_objectIDBufferId);
        glNamedBufferStorage(m_objectIDBufferId, preparedBufferSize * sizeof(glm::vec4), nullptr, GL_DYNAMIC_STORAGE_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, m_objectIDBufferId);

        glVertexAttribPointer(ATTR_INSTANCE_OBJECT_ID, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);

        glVertexAttribDivisor(ATTR_INSTANCE_OBJECT_ID, 1);

        glEnableVertexAttribArray(ATTR_INSTANCE_OBJECT_ID);
    }

    KI_DEBUG_SB("BATCHL: " << type->str() << " - model = " << m_modelBufferId << ", normal = " << m_normalBufferId << ", objectID = " << m_objectIDBufferId);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Batch::update(int count)
{
    if (batchSize <= 0) return;
    if (count <= 0) return;

    if (count > batchSize) {
        KI_WARN_SB("BATCH::CUT_OFF_BUFFER: count=" << count << " batchSize=" << batchSize);
        count = batchSize;
    }

    assert(m_modelMatrices.size() >= count);
    assert(count <= preparedBufferSize);

    assert(m_modelBufferId > 0);
    glNamedBufferSubData(m_modelBufferId, 0, count * sizeof(glm::mat4), &m_modelMatrices[0]);

    if (objectId) {
        assert(m_objectIDBufferId > 0);
        assert(m_objectIDs.size() >= count);
        glNamedBufferSubData(m_objectIDBufferId, 0, count * sizeof(glm::vec4), &m_objectIDs[0]);
    }
    else {
        assert(m_normalBufferId > 0);
        assert(m_normalMatrices.size() >= count);
        glNamedBufferSubData(m_normalBufferId, 0, count * sizeof(glm::mat3), &m_normalMatrices[0]);
    }

    //KI_GL_UNBIND(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Batch::bind(const RenderContext& ctx, Shader* shader)
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
    Node* node,
    Shader* shader)
{
    auto type = node->type.get();

    if (!type->mesh) return;

    //std::cout << node->type->mesh->modelName << '\n';
    if (node->groupId == KI_UUID("765f5288-21ec-4234-b7cd-6cdba4087e97"))
        int x = 0;

    const auto& volume = node->getVolume();
    if (ctx.assets.frustumEnabled && volume && !volume->isOnFrustum(ctx.frustum, node->getWorldModelMatrix())) {
        ctx.skipCount += 1;
        return;
    }

    ctx.drawCount += 1;

    if (batchSize == 0) {
        node->bind(ctx, shader);
        node->draw(ctx);
        return;
    }

    node->bindBatch(ctx, *this);

    if (m_modelMatrices.size() < batchSize) return;

    flush(ctx, type);
}

void Batch::flush(const RenderContext& ctx, NodeType* type)
{
    if (!type->mesh) return;

    if (batchSize == 0 || m_modelMatrices.empty()) return;

    int count = staticBuffer ? staticSize : m_modelMatrices.size();

    if (!staticBuffer) {
        update(count);
    }

    assert(count > 0);
    assert(count <= m_modelMatrices.size());
    assert(count <= preparedBufferSize);

    type->mesh->drawInstanced(ctx, count);

    if (!staticBuffer) {
        m_modelMatrices.clear();
        m_normalMatrices.clear();
        m_objectIDs.clear();
    }
}
