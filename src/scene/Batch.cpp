#include "Batch.h"

#include <mutex>
#include <fmt/format.h>

#include "glm/glm.hpp"

#include "ki/uuid.h"

#include "asset/VertexEntry.h"
#include "asset/Program.h"

#include "backend/gl/DrawIndirectCommand.h"

#include "backend/DrawRange.h"
#include "backend/DrawBuffer.h"

#include "model/Node.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/EntityRegistry.h"

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

bool Batch::inFrustumZ(
    const RenderContext& ctx,
    const int entityIndex)
{
    if (!m_frustumCPU) return true;
    if (entityIndex == -1) return false;

    const auto* entity = m_entityRegistry->getEntity(entityIndex);

    if ((entity->u_flags & ENTITY_NO_FRUSTUM_BIT) == ENTITY_NO_FRUSTUM_BIT)
        return true;

    const auto& volume = entity->u_volume;

    const glm::vec3 volumeCenter = glm::vec3(volume);
    const float volumeRadius = volume.a;

    const glm::mat4 projectedModel = ctx.m_matrices.u_projected * entity->getModelMatrix();

    const glm::vec4 centerPos = projectedModel * glm::vec4(volumeCenter, 1.0);
    const glm::vec3 radiusPos = projectedModel * glm::vec4(volumeCenter + glm::vec3(volumeRadius), 1.0);

    const float radius = glm::length(radiusPos - glm::vec3(centerPos));

    const float w = centerPos.w * 1.0;

    bool visible = -w <= centerPos.x + radius && centerPos.x - radius <= w &&
        -w <= centerPos.y + radius && centerPos.y - radius <= w &&
        -w <= centerPos.z + radius && centerPos.z - radius <= w;

    if (visible) {
        m_drawCount++;
    }
    else {
        m_skipCount++;
    }
    return visible;
}

void Batch::add(
    const RenderContext& ctx,
    const int entityIndex)
{
    if (entityIndex < 0) throw std::runtime_error{ "INVALID_ENTITY_INDEX" };

    if (!inFrustumZ(ctx, entityIndex))
        return;

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
    int instancedEntityIndex,
    const int firstEntityIndex,
    const int count)
{
    if (firstEntityIndex < 0 || count <= 0) return;

    int actualIndex = firstEntityIndex;
    int actualCount = count;

    if (m_frustumCPU) {
        if (instancedEntityIndex != -1 && !inFrustumZ(ctx, instancedEntityIndex)) {
            m_skipCount += count - 1;
            return;
        }

        while (!inFrustumZ(ctx, actualIndex) && actualCount > 0) {
            actualIndex++;
            actualCount--;
        }

        if (actualCount > 0) {
            int endIndex = actualIndex + actualCount;
            while (!inFrustumZ(ctx, endIndex) && actualCount > 0) {
                endIndex--;
                actualCount--;
            }
        }

        if (actualCount == 0)
            return;
    }

    auto& top = m_batches.back();

    top.m_drawCount = 1;
    top.m_instancedCount = actualCount;

    m_entityIndeces.push_back(actualIndex);
}

void Batch::bind() noexcept
{
    m_draw->bind();
}

void Batch::prepare(
    const Assets& assets,
    Registry* registry,
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
    m_draw->prepare(assets, registry, entryCount, bufferCount);

    m_frustumCPU = assets.frustumEnabled && assets.frustumCPU;
    m_frustumGPU = assets.frustumEnabled && assets.frustumGPU;

    m_entityRegistry = registry->m_entityRegistry.get();
}

void Batch::addCommand(
    const RenderContext& ctx,
    MeshType* type,
    Program* program)
{
    auto& cmd = m_batches.emplace_back();

    cmd.m_vao = type->m_vao;
    cmd.m_program = program;
    cmd.m_drawOptions = &type->m_drawOptions;
    cmd.m_index = m_entityIndeces.size();
}

void Batch::draw(
    const RenderContext& ctx,
    Node& node,
    Program* program)
{
    const auto type = node.m_type;

    if (!type->getMesh()) return;
    if (type->m_flags.invisible) return;
    if (type->m_flags.noDisplay) return;

    {
        const bool allowBlend = ctx.m_allowBlend;
        bool change = true;
        if (!m_batches.empty()) {
            auto& top = m_batches.back();
            change = program != top.m_program ||
                type->m_vao != top.m_vao ||
                !top.m_drawOptions->isSameDrawCommand(
                    type->m_drawOptions,
                    ctx.m_forceWireframe,
                    allowBlend);
        }

        if (change) {
            addCommand(ctx, type, program);
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
            curr.m_program,
            curr.m_vao,
            curr.m_drawOptions,
            ctx.m_allowBlend,
            ctx.m_forceWireframe
        };

        const auto& drawOptions = *curr.m_drawOptions;

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            backend::gl::DrawElementsIndirectCommand& cmd = indirect.element;

            cmd.count = drawOptions.indexCount;
            cmd.instanceCount = m_frustumGPU ? 0 : 1;
            cmd.firstIndex = drawOptions.indexOffset / sizeof(GLuint);
            cmd.baseVertex = drawOptions.vertexOffset / sizeof(VertexEntry);

            if (!m_frustumGPU && drawOptions.instanced) {
                cmd.instanceCount = curr.m_instancedCount;
                cmd.baseInstance = m_entityIndeces[curr.m_index];
                m_draw->send(drawRange, indirect);
            }
            else {
                for (int i = curr.m_index; i < curr.m_index + curr.m_drawCount; i++) {
                    for (int instanceIndex = 0; instanceIndex < curr.m_instancedCount; instanceIndex++) {
                        int entityIndex = m_entityIndeces[i] + instanceIndex;
                        cmd.baseInstance = entityIndex;
                        m_draw->send(drawRange, indirect);
                    }
                }
            }
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            backend::gl::DrawArraysIndirectCommand& cmd = indirect.array;

            cmd.vertexCount = drawOptions.indexCount;
            cmd.instanceCount = m_frustumGPU ? 0 : 1;
            cmd.firstVertex = drawOptions.indexOffset / sizeof(GLuint);

            if (!m_frustumGPU && drawOptions.instanced) {
                cmd.instanceCount = curr.m_instancedCount;
                cmd.baseInstance = m_entityIndeces[curr.m_index];
                m_draw->send(drawRange, indirect);
            } else {
                for (int i = curr.m_index; i < curr.m_index + curr.m_drawCount; i++) {
                    for (int instanceIndex = 0; instanceIndex < curr.m_instancedCount; instanceIndex++) {
                        int entityIndex = m_entityIndeces[i] + instanceIndex;
                        cmd.baseInstance = entityIndex;
                        m_draw->send(drawRange, indirect);
                    }
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

backend::gl::PerformanceCounters Batch::getCountersLocal(bool clear)
{
    backend::gl::PerformanceCounters counters{ m_drawCount, m_skipCount };
    if (clear) {
        m_drawCount = 0;
        m_skipCount = 0;
    }
    return counters;
}
