#include "Batch.h"

#include <mutex>
#include <fmt/format.h>

#include "glm/glm.hpp"

#include "ki/uuid.h"

#include "asset/VertexEntry.h"
#include "asset/Shader.h"

#include "backend/gl/DrawIndirectCommand.h"

#include "backend/DrawRange.h"
#include "backend/DrawBuffer.h"

#include "model/Node.h"
#include "registry/MeshType.h"

#include "scene/RenderContext.h"

namespace {
    constexpr int BATCH_COUNT = 100;
    constexpr int ENTITY_COUNT = 500000;
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

void Batch::addInstanced(
    const RenderContext& ctx,
    int firstIndex,
    int count)
{
    if (firstIndex < 0 || count <= 0) return;

    auto& top = m_batches.back();

    top.m_drawCount = 1;
    top.m_instancedCount = count;
    m_entityIndeces.push_back(firstIndex);
}

void Batch::bind() noexcept
{
    m_draw->bind();
}

void Batch::prepare(
    const Assets& assets,
    ShaderRegistry& shaders,
    int entryCount,
    int bufferCount)
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

    m_batches.reserve(BATCH_COUNT);
    m_entityIndeces.reserve(ENTITY_COUNT);

    m_draw = std::make_unique<backend::DrawBuffer>();
    m_draw->prepare(assets, shaders, entryCount, bufferCount);
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

    {
        const bool allowBlend = ctx.m_allowBlend;
        bool change = true;
        if (!m_batches.empty()) {
            auto& top = m_batches.back();
            change = shader != top.m_shader ||
                type->m_vao != top.m_vao ||
                !top.m_drawOptions->isSameDrawCommand(type->m_drawOptions, allowBlend);
        }

        if (change) {
            addCommand(ctx, type, shader);
        }

        auto& top = m_batches.back();
        top.m_materialVBO = &type->m_materialVBO;
    }

    node.bindBatch(ctx, *this);
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

    backend::gl::DrawIndirectCommand indirect{};

    // NOTE KI baseVertex usage
    // https://community.khronos.org/t/vertex-buffer-management-with-indirect-drawing/77272
    // https://www.khronos.org/opengl/wiki/Vertex_Specification#Instanced_arrays
    //

    for (auto& curr : m_batches) {
        if (curr.m_drawCount == 0) continue;

        backend::DrawRange drawRange = {
            &ctx.state,
            curr.m_shader,
            curr.m_vao,
            curr.m_drawOptions,
            ctx.m_allowBlend,
            ctx.m_forceWireframe
        };

        const auto& drawOptions = *curr.m_drawOptions;

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            backend::gl::DrawElementsIndirectCommand& cmd = indirect.element;

            cmd.count = drawOptions.indexCount;
            cmd.instanceCount = 0;
            cmd.firstIndex = drawOptions.indexOffset / sizeof(GLuint);
            cmd.baseVertex = drawOptions.vertexOffset / sizeof(VertexEntry);

            for (int i = curr.m_index; i < curr.m_index + curr.m_drawCount; i++) {
                for (int instanceIndex = 0; instanceIndex < curr.m_instancedCount; instanceIndex++) {
                    int entityIndex = m_entityIndeces[i] + instanceIndex;
                    cmd.baseInstance = entityIndex;
                    m_draw->send(drawRange, indirect);
                }
            }
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            backend::gl::DrawArraysIndirectCommand& cmd = indirect.array;

            cmd.vertexCount = drawOptions.indexCount;
            cmd.instanceCount = 0;
            cmd.firstVertex = drawOptions.indexOffset / sizeof(GLuint);

            for (int i = curr.m_index; i < curr.m_index + curr.m_drawCount; i++) {
                for (int instanceIndex = 0; instanceIndex < curr.m_instancedCount; instanceIndex++) {
                    int entityIndex = m_entityIndeces[i] + instanceIndex;
                    cmd.baseInstance = entityIndex;
                    m_draw->send(drawRange, indirect);
                }
            }
        }
        else {
            // NOTE KI "none" no drawing
            KI_INFO("no render");
        }
    }

    m_draw->flush();
    m_draw->drawPending(false);

    m_batches.clear();
    m_entityIndeces.clear();
}

backend::gl::PerformanceCounters Batch::getCounters(bool clear)
{
    return m_draw->getCounters(clear);
}
