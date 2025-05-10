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
#include "asset/Sphere.h"
#include "asset/Frustum.h"

#include "shader/Program.h"

#include "backend/gl/DrawIndirectCommand.h"
#include "backend/MultiDrawRange.h"
#include "backend/DrawBuffer.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"
#include "mesh/LodMesh.h"
#include "mesh/InstanceFlags.h"
#include "mesh/MeshTransform.h"

#include "model/Node.h"
#include "model/Snapshot.h"
#include "model/EntityFlags.h"

#include "registry/Registry.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "render/Camera.h"
#include "render/RenderContext.h"
#include "render/DebugContext.h"

#include "BatchCommand.h"

namespace {
    constexpr int ENTITY_COUNT = 100000;
    constexpr int BATCH_RANGE_COUNT = 8;

    constexpr int32_t SKIP{ -1 };
    // NOTE KI store accepted *INDECES*
    std::vector<int32_t> s_accept;
    std::vector<float> s_distances2;

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
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        const Snapshot& snapshot,
        uint32_t entityIndex) noexcept
    {
        if (entityIndex < 0) return;

        bool frustumChecked = !((snapshot.m_flags & ENTITY_NO_FRUSTUM_BIT) != ENTITY_NO_FRUSTUM_BIT);

        auto dist2 = glm::distance2(snapshot.getWorldPosition(), ctx.m_camera->getWorldPosition());

        for (const auto& lodMesh : *type->m_lodMeshes) {
            if (lodMesh.m_minDistance2 > dist2) continue;
            if (lodMesh.m_maxDistance2 <= dist2) continue;

            const auto& drawOptions = lodMesh.m_drawOptions;
            if (!drawOptions.isKind(kindBits)) continue;
            if (drawOptions.m_type == backend::DrawOptions::Type::none) continue;

            auto programId = programSelector(lodMesh);
            if (!programId) continue;

            if (!frustumChecked) {
                const auto& frustum = ctx.m_camera->getFrustum();
                if (m_frustumCPU && !inFrustum(frustum, snapshot.getVolume())) {
                    m_skipCount++;
                    return;
                }
                frustumChecked = true;
                m_drawCount++;
            }

            CommandEntry* commandEntry{ nullptr };
            {
                MultiDrawKey drawKey{
                    programId,
                    lodMesh.m_vaoId,
                    drawOptions
                };

                CommandKey commandKey{
                    lodMesh.m_baseVertex,
                    lodMesh.m_baseIndex,
                };

                const auto drawIndex = m_batchRegistry.getMultiDrawIndex(drawKey);
                const auto commandIndex = m_batchRegistry.getCommandIndex(commandKey);

                if (m_pending.size() < drawIndex + 1)
                {
                    m_pending.resize(drawIndex + 1);
                }

                auto& drawEntry = m_pending[drawIndex];
                drawEntry.m_index = drawIndex;

                if (drawEntry.m_commands.size() < commandIndex + 1)
                {
                    drawEntry.m_commands.resize(commandIndex + 1);
                }

                commandEntry = &drawEntry.m_commands[commandIndex];
                commandEntry->m_index = commandIndex;
                commandEntry->m_indexCount = lodMesh.m_indexCount;

                drawEntry.m_dirty = true;
            }

            commandEntry->addInstance({
                lodMesh.m_transform,
                dist2,
                entityIndex,
                static_cast<uint32_t>(lodMesh.m_materialIndex),
                lodMesh.m_socketIndex
                });

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
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        const Snapshot& snapshot,
        std::span<const mesh::MeshTransform> transforms,
        uint32_t entityIndex) noexcept
    {
        const uint32_t count = static_cast<uint32_t>(transforms.size());

        if (count <= 0) return;

        const auto& cameraPos = ctx.m_camera->getWorldPosition();

        bool useFrustum = m_frustumCPU;
        {
            if ((snapshot.m_flags & ENTITY_NO_FRUSTUM_BIT) == ENTITY_NO_FRUSTUM_BIT)
                useFrustum = false;
        }

        uint32_t instanceCount = count;

        {
            if (s_accept.size() < count) {
                s_accept.resize(count);
                s_distances2.resize(count);
            }
            for (uint32_t i = 0; i < count; i++) {
                s_accept[i] = i; // store index

                const auto& transform = transforms[i];
                s_distances2[i] = glm::distance2(transform.getWorldPosition(), cameraPos);
            }
        }

        const auto isValidLodMesh = [&kindBits, &programSelector] (
            const int instanceIndex,
            const float dist2,
            const auto& lodMesh) -> ki::program_id
            {
                if (lodMesh.m_minDistance2 > dist2) return 0;
                if (lodMesh.m_maxDistance2 <= dist2) return 0;

                const auto& drawOptions = lodMesh.m_drawOptions;
                if (drawOptions.m_type == backend::DrawOptions::Type::none) return 0;
                if (!drawOptions.isKind(kindBits)) return 0;

                return programSelector(lodMesh);
            };

        if (useFrustum) {
            const auto& frustum = ctx.m_camera->getFrustum();

            const auto& checkFrustum = [this, &type, &frustum, &transforms, &isValidLodMesh]
                (int32_t& idx)
                {
                    bool validMesh = false;
                    for (const auto& lodMesh : *type->m_lodMeshes) {
                        validMesh = isValidLodMesh(idx, s_distances2[idx], lodMesh) != 0;
                        if (validMesh) break;
                    }

                    if (!validMesh || !inFrustum(frustum, transforms[idx].getVolume())) {
                        idx = SKIP;
                    }
                };

            if (count > m_frustumParallelLimit) {
                std::for_each(
                    std::execution::par_unseq,
                    s_accept.begin(),
                    s_accept.begin() + count,
                    checkFrustum);
            }
            else {
                std::for_each(
                    std::execution::unseq,
                    s_accept.begin(),
                    s_accept.begin() + count,
                    checkFrustum);
            }
        }

        {
            for (uint32_t i = 0; i < count; i++) {
                if (useFrustum && s_accept[i] == SKIP) {
                    instanceCount--;
                    continue;
                }

                const auto dist2 = s_distances2[i];

                for (const auto& lodMesh : *type->m_lodMeshes) {
                    const auto  programId = isValidLodMesh(i, dist2, lodMesh);
                    if (!programId) continue;

                    //Program* program = Program::get(programId);

                    CommandEntry* commandEntry{ nullptr };
                    {
                        MultiDrawKey drawKey{
                            programId,
                            lodMesh.m_vaoId,
                            lodMesh.m_drawOptions
                        };

                        CommandKey commandKey{
                            lodMesh.m_baseVertex,
                            lodMesh.m_baseIndex,
                        };

                        const auto drawIndex = m_batchRegistry.getMultiDrawIndex(drawKey);
                        const auto commandIndex = m_batchRegistry.getCommandIndex(commandKey);

                        if (m_pending.size() < drawIndex + 1)
                        {
                            m_pending.resize(drawIndex + 1);
                        }

                        auto& drawEntry = m_pending[drawIndex];
                        drawEntry.m_index = drawIndex;

                        if (drawEntry.m_commands.size() < commandIndex + 1)
                        {
                            drawEntry.m_commands.resize(commandIndex + 1);
                        }

                        commandEntry = &drawEntry.m_commands[commandIndex];
                        commandEntry->m_index = commandIndex;
                        commandEntry->m_indexCount = lodMesh.m_indexCount;

                        drawEntry.m_dirty = true;
                    }

                    commandEntry->reserve(count);
                    commandEntry->addInstance({
                        transforms[i].getTransform() * lodMesh.m_transform,
                        dist2,
                        entityIndex,
                        static_cast<uint32_t>(lodMesh.m_materialIndex),
                        lodMesh.m_socketIndex
                        });

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

    void Batch::updateRT(
        const UpdateContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        const auto& dbg = ctx.m_dbg;

        m_frustumCPU = assets.frustumEnabled && assets.frustumCPU && dbg.m_frustumEnabled;

        m_pending.resize(m_batchRegistry.getMaxMultDrawIndex() + 1);
    }

    void Batch::draw(
        const RenderContext& ctx,
        mesh::MeshType* type,
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
        uint8_t kindBits,
        Node& node)
    {
        if (type->m_nodeType == NodeType::origo) return;
        if (type->m_flags.invisible || !node.m_visible) return;

        node.updateVAO(ctx);
        node.bindBatch(ctx, type, programSelector, kindBits, *this);
    }

    bool Batch::isFlushed() const noexcept
    {
        return m_pendingCount == 0;
    }

    void Batch::clearBatches() noexcept
    {
        int pendingIndex = -1;
        for (auto& multiDraw : m_pending) {
            pendingIndex++;
            multiDraw.clear();
        }
        m_instances.clear();
        m_pendingCount = 0;
    }

    size_t Batch::flush(
        const RenderContext& ctx)
    {
        if (m_pendingCount == 0) return 0;

        size_t flushCount = 0;

        // Sort instances
        // TODO KI this can slowdown things if lot of objects
        if (false) {
            for (auto& multiDraw : m_pending) {
                if (multiDraw.empty()) continue;

                for (auto& command : multiDraw.m_commands) {
                    if (command.m_instanceCount < 2) continue;

                    std::sort(
                        command.m_instances,
                        command.m_instances + command.m_instanceCount,
                        [](const auto& a, const auto& b) {
                            return a.m_distance2 < b.m_distance2;
                        });
                }
            }
        }

        // Setup instances
        {
            m_instances.clear();

            for (auto& multiDraw : m_pending)
            {
                if (multiDraw.empty()) continue;

                const auto& multiDrawKey = *m_batchRegistry.getMultiDraw(multiDraw.m_index);

                for (auto& command : multiDraw.m_commands)
                {
                    if (command.empty()) continue;

                    command.m_baseIndex = static_cast<uint32_t>(m_instances.size());

                    for (uint32_t i = 0; i < command.m_instanceCount; i++)
                    {
                        auto& lodEntry = command.m_instances[i];
                        auto& instance = m_instances.emplace_back();
                        instance.u_entityIndex = lodEntry.m_entityIndex;
                        instance.setTransform(
                            lodEntry.u_transformMatrixRow0,
                            lodEntry.u_transformMatrixRow1,
                            lodEntry.u_transformMatrixRow2);
                        instance.u_materialIndex = lodEntry.m_materialIndex;
                        instance.u_socketIndex = lodEntry.m_socketIndex;

                        // NOTE KI BatchKey does not take in account m_flags
                        // => can draw different instances in same batch
                        //instance.u_shapeIndex = key.m_drawOptions.m_flags;
                        instance.u_flags = multiDrawKey.m_drawOptions.m_flags;
                    }
                }
            }

            if (m_instances.empty()) {
                clearBatches();
                return 0;
            }
        }

        flushCount = m_instances.size();
        m_flushedTotalCount += flushCount;

        // NOTE KI baseVertex usage
        // https://community.khronos.org/t/vertex-buffer-management-with-indirect-drawing/77272
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Instanced_arrays
        //

        auto* draw = m_draw.get();

        draw->sendInstanceIndeces(m_instances);

        backend::gl::DrawIndirectCommand indirect{};

        const bool forceSolid = ctx.m_forceSolid;
        const bool forceLineMode = ctx.m_forceLineMode;

        for (const auto& multiDraw : m_pending) {
            if (multiDraw.empty()) continue;

            const auto& multiDrawKey = *m_batchRegistry.getMultiDraw(multiDraw.m_index);

            backend::MultiDrawRange drawRange = {
                multiDrawKey.m_drawOptions,
                multiDrawKey.m_vaoId,
                multiDrawKey.m_programId,
            };
            if (forceSolid) {
                drawRange.m_drawOptions.m_kindBits &= ~render::KIND_BLEND;
            }
            if (forceLineMode) {
                drawRange.m_drawOptions.m_lineMode = true;
            }

            const auto drawType = drawRange.m_drawOptions.m_type;

            for (const auto& command : multiDraw.m_commands) {
                if (command.empty()) continue;

                const auto& commandKey = *m_batchRegistry.getCommand(command.m_index);

                if (drawType == backend::DrawOptions::Type::elements) {
                    backend::gl::DrawElementsIndirectCommand& cmd = indirect.element;

                    cmd.u_instanceCount = command.m_instanceCount;
                    cmd.u_baseInstance = command.m_baseIndex;

                    cmd.u_baseVertex = commandKey.m_baseVertex;
                    cmd.u_firstIndex = commandKey.m_baseIndex;
                    cmd.u_count = command.m_indexCount;

                    //if (cmd.u_instanceCount > 1) {
                    //    KI_INFO_OUT(fmt::format("BATCH: element_instances={}", cmd.u_instanceCount));
                    //}

                    draw->send(drawRange, indirect);
                }
                else if (drawType == backend::DrawOptions::Type::arrays)
                {
                    backend::gl::DrawArraysIndirectCommand& cmd = indirect.array;

                    cmd.u_instanceCount = command.m_instanceCount;
                    cmd.u_baseInstance = command.m_baseIndex;

                    cmd.u_vertexCount = command.m_indexCount;
                    cmd.u_firstVertex = commandKey.m_baseIndex;

                    //if (cmd.u_instanceCount > 1) {
                    //    KI_INFO_OUT(fmt::format("BATCH: array_instances={}", cmd.u_instanceCount));
                    //}

                    draw->send(drawRange, indirect);
                }
                else {
                    // NOTE KI "none" no drawing
                    KI_INFO("no render");
                }
            }
        }

        {
            draw->flush();
            draw->drawPending(false);
            clearBatches();
        }

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
