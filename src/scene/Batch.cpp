#include "Batch.h"

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

    const glm::mat4 BASE_MAT_1 = glm::mat4(1.0f);
    const glm::vec3 SCALE{ 1.02f };
    const glm::mat4 SELECTION_MAT = glm::scale(BASE_MAT_1, SCALE);

    // scene_full = 91 109
    constexpr int MAX_MATERIAL_ENTRIES = 100000;
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

    auto& top = m_batches.back();
    top.m_drawCount += 1;

    // NOTE KI handles "instance" material case; per vertex separately
    entry.materialIndex = top.m_materialVBO->m_entries[0].materialIndex;

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
    m_drawCommands.reserve(count);
}

int Batch::size() noexcept
{
    return m_entries.size();
}

void Batch::clear() noexcept
{
    m_batches.clear();
    m_entries.clear();
    m_drawCommands.clear();
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

    {
        m_drawBuffer.create();
        m_drawBuffer.initEmpty(m_bufferSize * sizeof(backend::DrawIndirectCommand), GL_DYNAMIC_STORAGE_BIT);
    }
    {
        m_materialBuffer.create();
        m_materialBuffer.initEmpty(m_bufferSize * sizeof(backend::DrawIndirectCommand), GL_DYNAMIC_STORAGE_BIT);
    }

    m_entries.reserve(m_bufferSize);
    m_drawCommands.reserve(m_bufferSize);

    KI_DEBUG(fmt::format(
        "BATCHL: size={}, buffer={}",
        m_bufferSize, m_buffer));
}

void Batch::prepareVAO(
    GLVertexArray& vao,
    bool singleMaterial)
{
    KI_GL_CHECK("1");
    glVertexArrayVertexBuffer(vao, VBO_BATCH_BINDING, m_buffer, m_offset, sizeof(BatchEntry));
    KI_GL_CHECK("1.1");

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

    // material
    {
        glEnableVertexArrayAttrib(vao, ATTR_INSTANCE_MATERIAL_INDEX);
        glVertexArrayAttribFormat(vao, ATTR_INSTANCE_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(BatchEntry, materialIndex));
        glVertexArrayAttribBinding(vao, ATTR_INSTANCE_MATERIAL_INDEX, VBO_BATCH_BINDING);
    }

    {
        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.g-truc.net/post-0363.html
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_BATCH_BINDING, 1);
    }

    KI_GL_CHECK("2");

    if (!singleMaterial) {
        // TODO KI prepare material vbo separately
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
    m_buffer.update(m_offset, count * sizeof(BatchEntry), m_entries.data());
}

void Batch::updateCommands() noexcept
{
    m_drawBuffer.update(0, m_drawCommands.size() * sizeof(backend::DrawIndirectCommand), m_drawCommands.data());
}

void Batch::bind(
    const RenderContext& ctx,
    MeshType* type,
    Shader* shader)
{
    BatchCommand cmd;
    cmd.m_vao = type->m_vao;
    cmd.m_shader = shader;
    cmd.m_drawOptions = type->m_drawOptions;

    m_batches.push_back(cmd);
}

void Batch::draw(
    const RenderContext& ctx,
    Node& node,
    Shader* shader)
{
    const auto type = node.m_type;

    if (!type->getMesh()) return;
    if (type->m_flags.noRender) return;

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

    bool needBind = true;
    if (!m_batches.empty()) {
        auto& top = m_batches.back();
        needBind = !top.m_drawOptions.isSameDrawCommand(type->m_drawOptions);
    }

    if (needBind) {
        bind(ctx, type, shader);
    }
    auto& top = m_batches.back();
    top.m_materialVBO = &type->m_materialVBO;

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
    int batchCount = m_batches.size();

    if (batchCount == 0) {
        if (release) {
            clear();
        }
        return;
    }

    update(m_entries.size());

    drawInstanced(ctx);

    m_entries.clear();

    if (release) {
        clear();
    }
}

void Batch::drawInstanced(
    const RenderContext& ctx)
{
    Shader* boundShader{ nullptr };
    GLVertexArray* boundVAO{ nullptr };
    backend::DrawOptions boundDrawOptions;
    int  baseInstance = 0;

    for (auto& curr : m_batches) {
        if (ctx.assets.glDebug) {
            curr.m_shader->validateProgram();
        }

        bool sameDraw = boundShader == curr.m_shader &&
            boundVAO == curr.m_vao &&
            boundDrawOptions.isSameMultiDraw(curr.m_drawOptions);

        if (!sameDraw) {
            drawPending(ctx, boundShader, boundVAO, boundDrawOptions);

            boundShader = curr.m_shader;
            boundVAO = curr.m_vao;
            boundDrawOptions = curr.m_drawOptions;
        }

        // NOTE KI baseVertex usage
        // https://community.khronos.org/t/vertex-buffer-management-with-indirect-drawing/77272
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Instanced_arrays
        //
        auto& drawOptions = curr.m_drawOptions;

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            backend::DrawIndirectCommand indirect;
            backend::DrawElementsIndirectCommand& cmd = indirect.element;

            cmd.count = drawOptions.indexCount;
            cmd.instanceCount = curr.m_drawCount;
            cmd.firstIndex = drawOptions.indexOffset / sizeof(GLuint);
            cmd.baseVertex = drawOptions.vertexOffset / sizeof(VertexEntry);
            //cmd.firstIndex = 0;
            //cmd.baseVertex = drawOptions.indexOffset;
            cmd.baseInstance = baseInstance;

            m_drawCommands.push_back(indirect);
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            backend::DrawIndirectCommand indirect;
            backend::DrawArraysIndirectCommand& cmd = indirect.array;

            cmd.vertexCount = drawOptions.indexCount;
            cmd.instanceCount = curr.m_drawCount;
            cmd.firstVertex = drawOptions.indexOffset / sizeof(GLuint);
            cmd.baseInstance = baseInstance;

            m_drawCommands.push_back(indirect);
        }
        else {
            // NOTE KI "none" no drawing
            KI_INFO("no render");
        }

        baseInstance += curr.m_drawCount;
    }

    drawPending(ctx, boundShader, boundVAO, boundDrawOptions);

    m_batches.clear();
}

void Batch::drawPending(
    const RenderContext& ctx,
    Shader* shader,
    GLVertexArray* vao,
    backend::DrawOptions drawOptions)
{
    if (m_drawCommands.empty()) return;

    updateCommands();
    m_drawBuffer.bindDrawIndirect();

    ctx.bindDraw(drawOptions.renderBack, drawOptions.wireframe);
    shader->bind(ctx.state);
    ctx.state.useVAO(*vao);

    if (drawOptions.type == backend::DrawOptions::Type::elements) {
        glMultiDrawElementsIndirect(
            drawOptions.mode,
            GL_UNSIGNED_INT,
            0,
            m_drawCommands.size(),
            sizeof(backend::DrawIndirectCommand));
    }
    else if (drawOptions.type == backend::DrawOptions::Type::arrays)
    {
        glMultiDrawArraysIndirect(
            drawOptions.mode,
            0,
            m_drawCommands.size(),
            sizeof(backend::DrawIndirectCommand));
    }

    m_drawCommands.clear();
}
