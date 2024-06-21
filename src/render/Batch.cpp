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
#include "backend/Lod.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"
#include "mesh/LodMesh.h"
#include "mesh/InstanceFlags.h"

#include "model/Node.h"
#include "model/Snapshot.h"
#include "model/EntityFlags.h"

#include "component/Camera.h"

#include "registry/Registry.h"

#include "engine/PrepareContext.h"
#include "render/RenderContext.h"

#include "BatchCommand.h"

namespace {
    constexpr int ENTITY_COUNT = 100000;
    constexpr int BATCH_RANGE_COUNT = 8;

    std::vector<uint32_t> s_accept;

    inline bool inFrustum(
        const Frustum& frustum,
        const glm::vec4& volume) noexcept
    {
        const Sphere& sphere{ volume };
        return sphere.isOnFrustum(frustum);
    }
}

namespace render {
    Batch::Batch()
    {
    }

    Batch::~Batch() = default;


    void Batch::addSnapshot(
        const RenderContext& ctx,
        const mesh::MeshType* type,
        const std::function<Program* (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        const Snapshot& snapshot,
        uint32_t entityIndex) noexcept
    {
        if ((snapshot.m_flags & ENTITY_NO_FRUSTUM_BIT) == ENTITY_NO_FRUSTUM_BIT)
            return;

        if (entityIndex < 0) return;

        const auto& frustum = ctx.m_camera->getFrustum();

        if (m_frustumCPU && !inFrustum(frustum, snapshot.m_volume)) {
            m_skipCount++;
            return;
        }

        m_drawCount++;

        const auto& cameraPos = ctx.m_camera->getWorldPosition();
        const auto lodLevel = type->getLodLevel(cameraPos, snapshot.m_worldPos);

        for (const auto& lodMesh : *type->m_lodMeshes) {
            if (lodMesh.m_level != lodLevel) continue;
            if (!lodMesh.m_vao) continue;

            const auto& drawOptions = lodMesh.m_drawOptions;
            if (!drawOptions.isKind(kindBits)) continue;
            if (drawOptions.m_type == backend::DrawOptions::Type::none) continue;

            auto* program = programSelector(lodMesh);
            if (!program) continue;

            BatchCommand* top;
            {
                BatchKey key{
                    lodMesh.m_priority,
                    program,
                    lodMesh.m_vao,
                    drawOptions,
                    ctx.m_forceSolid,
                    ctx.m_forceWireframe,
                };

                const auto& it = m_batches.find(key);
                if (it == m_batches.end()) {
                    const auto& pair = m_batches.insert({ key, {} });
                    top = &pair.first->second;
                } else {
                    top = &it->second;
                }
            }

            auto& lodInstances = top->m_lodInstances[{ &lodMesh.m_lod }];
            lodInstances.reserve(100);
            lodInstances.emplace_back(entityIndex, lodMesh.m_meshIndex);
            m_pendingCount++;
        }
    }

    //void Batch::addSnapshots(
    //    const RenderContext& ctx,
    //    mesh::MeshType* type,
    //    std::span<const Snapshot> snapshots,
    //    std::span<uint32_t> entityIndeces) noexcept
    //{
    //    uint32_t i = 0;
    //    for (const auto& snapshot : snapshots) {
    //        addSnapshot(ctx, type, snapshot, entityIndeces[i++]);
    //    }
    //}

    void Batch::addSnapshotsInstanced(
        const RenderContext& ctx,
        const mesh::MeshType* type,
        const std::function<Program* (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        std::span<const Snapshot> snapshots,
        uint32_t entityBaseIndex) noexcept
    {
        const uint32_t count = static_cast<uint32_t>(snapshots.size());

        if (count <= 0) return;

        const auto& cameraPos = ctx.m_camera->getWorldPosition();

        bool useFrustum = m_frustumCPU;
        {
            auto& snapshot = snapshots[0];
            if ((snapshot.m_flags & ENTITY_NO_FRUSTUM_BIT) == ENTITY_NO_FRUSTUM_BIT)
                useFrustum = false;
        }

        uint32_t instanceCount = count;

        if (useFrustum) {
            s_accept.reserve(snapshots.size());
            s_accept.clear();
            for (uint32_t i = 0; i < count; i++) {
                s_accept.push_back(i);
            }

            const auto& frustum = ctx.m_camera->getFrustum();

            if (count > m_frustumParallelLimit) {
                std::for_each(
                    std::execution::par_unseq,
                    s_accept.begin(),
                    s_accept.end(),
                    [this, &frustum, &snapshots](uint32_t& idx) {
                        if (!inFrustum(frustum, snapshots[idx].m_volume))
                            idx = -1;
                    });
            }
            else {
                std::for_each(
                    std::execution::unseq,
                    s_accept.begin(),
                    s_accept.end(),
                    [this, &frustum, &snapshots](uint32_t& idx) {
                        if (!inFrustum(frustum, snapshots[idx].m_volume))
                            idx = -1;
                    });
            }
        }

        {
            for (uint32_t i = 0; i < count; i++) {
                if (useFrustum && s_accept[i] == -1) {
                    instanceCount--;
                    continue;
                }

                const auto lodLevel = type->getLodLevel(cameraPos, snapshots[i].m_worldPos);

                for (const auto& lodMesh : *type->m_lodMeshes) {
                    if (lodMesh.m_level != lodLevel) continue;
                    if (!lodMesh.m_vao) continue;

                    const auto& drawOptions = lodMesh.m_drawOptions;
                    if (!drawOptions.isKind(kindBits)) continue;
                    if (drawOptions.m_type == backend::DrawOptions::Type::none) continue;

                    auto* program = programSelector(lodMesh);
                    if (!program) continue;

                    BatchCommand* top;
                    {
                        BatchKey key{
                            lodMesh.m_priority,
                            program,
                            lodMesh.m_vao,
                            drawOptions,
                            ctx.m_forceSolid,
                            ctx.m_forceWireframe,
                        };

                        const auto& it = m_batches.find(key);
                        if (it == m_batches.end()) {
                            const auto& pair = m_batches.insert({ key, {} });
                            top = &pair.first->second;
                        }
                        else {
                            top = &it->second;
                        }
                    }

                    auto& lodInstances = top->m_lodInstances[{ &lodMesh.m_lod }];
                    lodInstances.reserve(100);
                    lodInstances.emplace_back(entityBaseIndex + i, lodMesh.m_meshIndex);
                    m_pendingCount++;
                }
            }

            //std::cout << "instances: " << instanceCount << ", orig: " << count << '\n';

            m_skipCount += count - instanceCount;
            m_drawCount += instanceCount;
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

        const auto& assets = ctx.m_assets;

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

    void Batch::draw(
        const RenderContext& ctx,
        mesh::MeshType* type,
        const std::function<Program* (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        Node& node)
    {
        if (type->m_entityType == EntityType::origo) return;
        if (type->m_flags.invisible || !node.m_visible) return;

        node.updateVAO(ctx);
        node.bindBatch(ctx, type, programSelector, kindBits, *this);
    }

    bool Batch::isFlushed() const noexcept
    {
        //return m_batches.empty();
        return m_pendingCount == 0;
    }

    void Batch::clearBatches() noexcept
    {
        //KI_INFO_OUT(fmt::format("batches: {}, indeces={}", m_batches.size(), m_entityIndeces.size()));
        for (auto& batchIt : m_batches) {
            auto& batch = batchIt.second;
            for (auto& lodIt : batch.m_lodInstances) {
                auto& lods = lodIt.second;
                lods.clear();
            }
        }
        m_entityIndeces.clear();
        m_pendingCount = 0;
    }

    size_t Batch::flush(
        const RenderContext& ctx)
    {
        size_t flushCount = 0;

        std::map<const Program*, std::map<LodKey, uint32_t>> programLodBaseIndex;

        {
            m_entityIndeces.clear();
            for (const auto& it : m_batches) {
                const auto& key = it.first;
                const auto& curr = it.second;

                auto& lodBaseIndex = programLodBaseIndex[key.m_program];

                for (const auto& lodInstance : curr.m_lodInstances) {
                    const auto* lod = lodInstance.first.m_lod;
                    const auto& lodEntries = lodInstance.second;
                    if (lodEntries.empty()) continue;

                    lodBaseIndex[lodInstance.first] = static_cast<uint32_t>(m_entityIndeces.size());
                    for (auto& lodEntry : lodEntries) {
                        auto& instance = m_entityIndeces.emplace_back();
                        instance.u_entityIndex = lodEntry.m_entityIndex;
                        instance.u_meshIndex = lodEntry.m_meshIndex;
                        instance.u_materialIndex = lod->m_materialIndex;
                        instance.u_shapeIndex = key.m_drawOptions.m_flags;
                    }
                }
            }

            if (m_entityIndeces.empty()) {
                clearBatches();
                return 0;
            }
        }
        flushCount = m_entityIndeces.size();
        m_flushedTotalCount += flushCount;

        // NOTE KI baseVertex usage
        // https://community.khronos.org/t/vertex-buffer-management-with-indirect-drawing/77272
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Instanced_arrays
        //

        auto* draw = m_draw.get();

        draw->sendInstanceIndeces(m_entityIndeces);

        backend::gl::DrawIndirectCommand indirect{};

        for (const auto& it : m_batches) {
            const auto& key = it.first;
            const auto& curr = it.second;

            backend::DrawRange drawRange = {
                key.m_vao,
                key.m_program,
                key.m_drawOptions,
            };

            const auto& drawOptions = key.m_drawOptions;

            for (const auto& lodEntry : curr.m_lodInstances) {
                auto& lodBaseIndex = programLodBaseIndex[key.m_program];
                auto baseInstance = lodBaseIndex[lodEntry.first];

                const auto& lodEntries = lodEntry.second;
                if (lodEntries.empty()) continue;

                GLuint instanceCount = static_cast<GLuint>(lodEntries.size());
                const auto* lod = lodEntry.first.m_lod;

                if (drawOptions.m_type == backend::DrawOptions::Type::elements) {
                    backend::gl::DrawElementsIndirectCommand& cmd = indirect.element;

                    //cmd.u_instanceCount = m_frustumGPU ? 0 : 1;
                    cmd.u_instanceCount = instanceCount;
                    cmd.u_baseInstance = baseInstance;

                    cmd.u_baseVertex = lod->m_baseVertex;
                    cmd.u_firstIndex = lod->m_baseIndex;
                    cmd.u_count = lod->m_indexCount;

                    //KI_INFO_OUT(fmt::format("draw: {}", instanceCount));
                    draw->sendDirect(drawRange, indirect);
                }
                else if (drawOptions.m_type == backend::DrawOptions::Type::arrays)
                {
                    backend::gl::DrawArraysIndirectCommand& cmd = indirect.array;

                    //cmd.u_instanceCount = m_frustumGPU ? 0 : 1;
                    cmd.u_instanceCount = instanceCount;
                    cmd.u_baseInstance = baseInstance;

                    cmd.u_vertexCount = lod->m_indexCount;
                    cmd.u_firstVertex = lod->m_baseIndex;

                    draw->sendDirect(drawRange, indirect);
                }
                else {
                    // NOTE KI "none" no drawing
                    KI_INFO("no render");
                }
            }
        }

        draw->flush();
        draw->drawPending(false);

        clearBatches();

        return flushCount;
    }

    backend::gl::PerformanceCounters Batch::getCounters(bool clear) const
    {
        return m_draw->getCounters(clear);
    }

    backend::gl::PerformanceCounters Batch::getCountersLocal(bool clear) const
    {
        backend::gl::PerformanceCounters counters{
            static_cast<GLuint>(m_drawCount),
            static_cast<GLuint>(m_skipCount) };

        if (clear) {
            m_drawCount = 0;
            m_skipCount = 0;
        }
        return counters;
    }
}
