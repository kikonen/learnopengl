#include "Batch.h"
#include "Batch.h"

#include <mutex>
#include <iostream>
#include <algorithm>
#include <execution>

#include <fmt/format.h>

#include "glm/glm.hpp"


#include "util/glm_format.h"

#include "asset/Assets.h"
#include "asset/Program.h"
#include "asset/Sphere.h"
#include "asset/Frustum.h"

#include "backend/gl/DrawIndirectCommand.h"
#include "backend/DrawRange.h"
#include "backend/DrawBuffer.h"

#include "mesh/MeshType.h"
#include "mesh/PositionEntry.h"

#include "model/Node.h"
#include "model/Snapshot.h"
#include "model/EntityFlags.h"

#include "component/Camera.h"

#include "registry/Registry.h"

#include "engine/PrepareContext.h"
#include "render/RenderContext.h"


namespace {
    constexpr int BATCH_COUNT = 100;
    constexpr int ENTITY_COUNT = 100000;
    constexpr int BATCH_RANGE_COUNT = 8;

    std::vector<uint32_t> accept;
}

namespace render {
    Batch::Batch()
    {
    }

    Batch::~Batch() = default;

    bool Batch::inFrustum(
        const RenderContext& ctx,
        const Snapshot& snapshot) const noexcept
    {
        if ((snapshot.m_flags & ENTITY_NO_FRUSTUM_BIT) == ENTITY_NO_FRUSTUM_BIT)
            return true;

        bool visible;
        {
            const auto& frustum = ctx.m_camera->getFrustum();
            const Sphere& volume{ snapshot.m_volume };

            visible = volume.isOnFrustum(frustum);
        }

        if (visible) {
            m_drawCount++;
        }
        else {
            m_skipCount++;
        }

        return visible;
    }

    void Batch::addSnapshot(
        const RenderContext& ctx,
        const Snapshot& snapshot,
        uint32_t entityIndex) noexcept
    {
        //if (entityIndex < 0) throw std::runtime_error{ "INVALID_ENTITY_INDEX" };
        if (entityIndex < 0) return;

        if (m_frustumCPU && !inFrustum(ctx, snapshot))
            return;

        auto& top = m_batches.back();
        top.m_drawCount++;

        m_entityIndeces.emplace_back(entityIndex);
    }

    void Batch::addSnapshots(
        const RenderContext& ctx,
        const std::span<const Snapshot>& snapshots,
        const std::span<uint32_t>& entityIndeces) noexcept
    {
        uint32_t i = 0;
        for (const auto& snapshot : snapshots) {
            addSnapshot(ctx, snapshot, entityIndeces[i++]);
        }
    }

    void Batch::addSnapshotsInstanced(
        const RenderContext& ctx,
        const std::span<const Snapshot>& snapshots,
        uint32_t entityBase) noexcept
    {
        if (false) {
            const uint32_t count = static_cast<uint32_t>(snapshots.size());

            if (count <= 0) return;

            uint32_t startIndex = 0;
            uint32_t instanceCount = count;

            if (m_frustumCPU) {
                while (instanceCount > 0 && !inFrustum(ctx, snapshots[startIndex])) {
                    startIndex++;
                    instanceCount--;
                }

                if (instanceCount > 0) {
                    uint32_t endIndex = static_cast<uint32_t>(snapshots.size() - 1);
                    while (instanceCount > 0 && !inFrustum(ctx, snapshots[endIndex])) {
                        endIndex--;
                        instanceCount--;
                    }
                }

                if (instanceCount == 0)
                    return;
            }

            auto& top = m_batches.back();

            top.m_drawCount = 1;
            top.m_instancedCount = static_cast<int>(instanceCount);

            for (uint32_t i = 0; i < instanceCount; i++) {
                m_entityIndeces.emplace_back(entityBase + startIndex + i);
            }
        }
        else {
            const uint32_t count = static_cast<uint32_t>(snapshots.size());

            if (count <= 0) return;

            if (m_frustumCPU) {
                uint32_t instanceCount = count;

                accept.reserve(snapshots.size());
                accept.clear();
                for (uint32_t i = 0; i < count; i++) {
                    accept.push_back(i);
                }

                if (count > m_frustumParallelLimit) {
                    std::for_each(
                        std::execution::par_unseq,
                        accept.begin(),
                        accept.end(),
                        [this, &ctx, &snapshots](uint32_t& idx) {
                            if (!inFrustum(ctx, snapshots[idx]))
                                idx = -1;
                        });
                }
                else {
                    std::for_each(
                        std::execution::unseq,
                        accept.begin(),
                        accept.end(),
                        [this, &ctx, &snapshots](uint32_t& idx) {
                            if (!inFrustum(ctx, snapshots[idx]))
                                idx = -1;
                        });
                }

                for (uint32_t i = 0; i < count; i++) {
                    if (accept[i] == -1) {
                        instanceCount--;
                        continue;
                    }
                    m_entityIndeces.emplace_back(entityBase + i);
                }

                //std::cout << "instances: " << instanceCount << ", orig: " << count << '\n';

                if (instanceCount == 0)
                    return;

                auto& top = m_batches.back();

                top.m_drawCount = 1;
                top.m_instancedCount = static_cast<int>(instanceCount);
            }
            else {
                auto& top = m_batches.back();

                top.m_drawCount = 1;
                top.m_instancedCount = static_cast<int>(count);

                for (uint32_t i = 0; i < count; i++) {
                    m_entityIndeces.emplace_back(entityBase + i);
                }
            }
        }
    }

    void Batch::bind() noexcept
    {
        m_draw->bind();
    }

    void Batch::prepareRT(
        const PrepareContext& ctx,
        int entryCount,
        int bufferCount)
    {
        if (m_prepared) return;
        m_prepared = true;

        const auto& assets = Assets::get();

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

        m_draw = std::make_unique<backend::DrawBuffer>(
            assets.glUseMapped,
            assets.glUseInvalidate,
            assets.glUseFence,
            assets.glUseSingleFence,
            assets.glUseDebugFence);

        m_draw->prepareRT(ctx, entryCount, bufferCount);

        m_frustumCPU = assets.frustumEnabled && assets.frustumCPU;
        m_frustumGPU = assets.frustumEnabled && assets.frustumGPU;
        m_frustumParallelLimit = assets.frustumParallelLimit;
    }

    void Batch::addCommand(
        const RenderContext& ctx,
        const kigl::GLVertexArray* vao,
        const backend::DrawOptions& drawOptions,
        Program* program) noexcept
    {
        auto& cmd = m_batches.emplace_back();

        cmd.m_vao = vao;
        cmd.m_program = program;
        cmd.m_drawOptions = drawOptions;
        cmd.m_index = static_cast<int>(m_entityIndeces.size());
    }

    void Batch::draw(
        const RenderContext& ctx,
        Node& node,
        Program* program)
    {
        auto* type = node.m_typeHandle.toType();

        if (type->m_flags.invisible || !node.m_visible) return;

        node.updateVAO(ctx);

        const auto& vao = node.getVAO();
        if (!vao) return;

        {
            const auto& drawOptions = node.getDrawOptions();
            const bool allowBlend = ctx.m_allowBlend;

            bool change = true;
            if (!m_batches.empty()) {
                auto& top = m_batches.back();

                change = program != top.m_program ||
                    vao != top.m_vao ||
                    !top.m_drawOptions.isSameDrawCommand(
                        drawOptions,
                        ctx.m_forceWireframe,
                        allowBlend);
            }

            if (change) {
                addCommand(ctx, vao, drawOptions, program);
            }

            auto& top = m_batches.back();
        }

        node.bindBatch(ctx, *this);
    }

    void Batch::flush(
        const RenderContext& ctx)
    {
        // NOTE KI two cases
        // - empty batch
        // - "save back" entry without actual draw
        size_t pendingCount = m_entityIndeces.size();

        if (pendingCount == 0) {
            m_batches.clear();
            return;
        }

        backend::gl::DrawIndirectCommand indirect{};

        // NOTE KI baseVertex usage
        // https://community.khronos.org/t/vertex-buffer-management-with-indirect-drawing/77272
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Instanced_arrays
        //

        auto* draw = m_draw.get();

        draw->sendInstanceIndeces(m_entityIndeces);

        for (auto& curr : m_batches) {
            if (curr.m_drawCount == 0) continue;

            backend::DrawRange drawRange = {
                curr.m_program,
                curr.m_vao,
                curr.m_drawOptions,
                ctx.m_allowBlend,
                ctx.m_forceWireframe
            };

            const auto& drawOptions = curr.m_drawOptions;

            if (drawOptions.m_type == backend::DrawOptions::Type::elements) {
                backend::gl::DrawElementsIndirectCommand& cmd = indirect.element;

                cmd.u_count = drawOptions.m_indexCount;
                cmd.u_instanceCount = m_frustumGPU ? 0 : 1;
                cmd.u_firstIndex = drawOptions.m_indexOffset / sizeof(GLuint);
                cmd.u_baseVertex = drawOptions.m_vertexOffset / sizeof(mesh::PositionEntry);

                if (drawOptions.m_instanced) {
                    cmd.u_instanceCount = curr.m_instancedCount;
                    cmd.u_baseInstance = curr.m_index;
                    draw->sendDirect(drawRange, indirect);
                }
                else {
                    cmd.u_instanceCount = curr.m_drawCount;
                    cmd.u_baseInstance = curr.m_index;
                    draw->sendDirect(drawRange, indirect);
                }
            }
            else if (drawOptions.m_type == backend::DrawOptions::Type::arrays)
            {
                backend::gl::DrawArraysIndirectCommand& cmd = indirect.array;

                cmd.u_vertexCount = drawOptions.m_indexCount;
                cmd.u_instanceCount = m_frustumGPU ? 0 : 1;
                cmd.u_firstVertex = drawOptions.m_indexOffset / sizeof(GLuint);

                if (drawOptions.m_instanced) {
                    cmd.u_instanceCount = curr.m_instancedCount;
                    cmd.u_baseInstance = curr.m_index;
                    draw->sendDirect(drawRange, indirect);
                }
                else {
                    cmd.u_instanceCount = curr.m_drawCount;
                    cmd.u_baseInstance = curr.m_index;
                    draw->sendDirect(drawRange, indirect);
                }
            }
            else {
                // NOTE KI "none" no drawing
                KI_INFO("no render");
            }
        }

        draw->flush();
        draw->drawPending(false);

        m_batches.clear();
        m_entityIndeces.clear();
    }

    backend::gl::PerformanceCounters Batch::getCounters(bool clear) const
    {
        return m_draw->getCounters(clear);
    }

    backend::gl::PerformanceCounters Batch::getCountersLocal(bool clear) const
    {
        backend::gl::PerformanceCounters counters{ m_drawCount, m_skipCount };
        if (clear) {
            m_drawCount = 0;
            m_skipCount = 0;
        }
        return counters;
    }
}
