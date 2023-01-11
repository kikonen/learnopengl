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
    constexpr int BATCH_COUNT = 100;
    constexpr int ENTITY_COUNT = 100000;
    constexpr int BATCH_RANGE_COUNT = 8;

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

    auto& top = m_batches.back();
    top.m_drawCount++;
    m_entityIndeces.push_back(entityIndex);
}

void Batch::addAll(
    const RenderContext& ctx,
    const std::vector<int> entityIndeces)
{
    for (const auto& entityIndex : entityIndeces) {
        add(ctx, entityIndex);
    }
}

void Batch::bind() noexcept
{
    m_draw.bind();
}

void Batch::prepare(
    const Assets& assets,
    int entryCount,
    int bufferCount) noexcept
{
    if (m_prepared) return;
    m_prepared = true;

    if (entryCount <= 0) {
        entryCount = assets.batchSize;
    }
    if (entryCount <= 0) {
        entryCount = 1;
    }
    if (bufferCount <= 0) {
        bufferCount = assets.batchBuffers;
    }
    if (bufferCount <= 0) {
        bufferCount = 1;
    }
    m_entryCount = entryCount;

    m_batches.reserve(BATCH_COUNT);
    m_entityIndeces.reserve(ENTITY_COUNT);

    m_draw.prepare(m_entryCount, bufferCount);

    KI_DEBUG(fmt::format(
        "BATCHL: size={}, buffer={}",
        m_entryCount, -1));
}

void Batch::prepareVAO(
    GLVertexArray& vao,
    bool singleMaterial)
{
    // NOTE KI obsolete
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
    cmd.m_index = m_entityIndeces.size();
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
            node.getModelMatrix()))
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
}

void Batch::sendDraw(
    const RenderContext& ctx,
    const backend::DrawIndirectCommand& indirect,
    const Shader* shader,
    const GLVertexArray* vao,
    const backend::DrawOptions& drawOptions)
{
    m_draw.send(indirect);
    m_draw.flushIfNeeded(ctx.state, shader, vao, drawOptions, ctx.m_useBlend);
}

void Batch::flush(
    const RenderContext& ctx)
{
    // NOTE KI two cases
    // - empty batch
    // - "save back" entry without actual draw
    int pendingCount = m_entityIndeces.size();

    if (pendingCount == 0) {
        m_batches.clear();
        return;
    }

    const bool useBlend = ctx.m_useBlend;
    const Shader* boundShader{ nullptr };
    const GLVertexArray* boundVAO{ nullptr };
    const backend::DrawOptions* boundDrawOptions{ nullptr };
    //int  baseInstance = range.m_baseIndex;

    backend::DrawIndirectCommand indirect{};

    for (auto& curr : m_batches) {
        if (curr.m_drawCount == 0) continue;

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
            cmd.instanceCount = 1;
            cmd.firstIndex = drawOptions.indexOffset / sizeof(GLuint);
            cmd.baseVertex = drawOptions.vertexOffset / sizeof(VertexEntry);

            if (drawOptions.instanced) {
                cmd.instanceCount = curr.m_drawCount;
                // NOTE KI *SAME* e
                cmd.baseInstance = m_entityIndeces[curr.m_index];
                m_draw.send(indirect);
                m_draw.flushIfNeeded(ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
            }
            else {
                for (int i = curr.m_index; i < curr.m_index + curr.m_drawCount; i++) {
                    cmd.baseInstance = m_entityIndeces[i];
                    m_draw.send(indirect);
                    m_draw.flushIfNeeded(ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
                }
            }
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            backend::DrawArraysIndirectCommand& cmd = indirect.array;

            cmd.vertexCount = drawOptions.indexCount;
            cmd.instanceCount = 1;
            cmd.firstVertex = drawOptions.indexOffset / sizeof(GLuint);

            if (drawOptions.instanced) {
                cmd.instanceCount = curr.m_drawCount;
                cmd.baseInstance = m_entityIndeces[curr.m_index];
                m_draw.send(indirect);
                m_draw.flushIfNeeded(ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
            }
            else {
                for (int i = curr.m_index; i < curr.m_index + curr.m_drawCount; i++) {
                    cmd.baseInstance = m_entityIndeces[i];
                    m_draw.send(indirect);
                    m_draw.flushIfNeeded(ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
                }
            }
        }
        else {
            // NOTE KI "none" no drawing
            KI_INFO("no render");
        }
    }

    if (boundShader) {
        m_draw.flush(ctx.state, boundShader, boundVAO, *boundDrawOptions, useBlend);
    }

    m_batches.clear();
    m_entityIndeces.clear();
}
