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

#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"
#include "mesh/Transform.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/Snapshot.h"
#include "model/EntityFlags.h"

#include "registry/Registry.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "render/Camera.h"
#include "render/RenderContext.h"
#include "debug/DebugContext.h"

#include "render/InstanceFlags.h"
#include "render/InstanceRegistry.h"
#include "render/InstanceSSBO.h"
#include "render/InstanceIndexSSBO.h"
#include "render/BatchCommand.h"

namespace {
    constexpr int ENTITY_COUNT = 100000;
    //constexpr int BATCH_RANGE_COUNT = 8;

    inline glm::mat4 ID_MAT{ 1.f };

    constexpr int32_t SKIP{ -1 };
    // NOTE KI store accepted *INDECES*
    std::vector<int32_t> s_accept;
    std::vector<float> s_distances2;

    //inline bool inFrustum(
    //    const Frustum& frustum,
    //    const SphereVolume& worldVolume) noexcept
    //{
    //    worldVolume.isOnFrustum(frustum);
    //}
}

namespace render {
    Batch::Batch()
    {
    }

    Batch::~Batch() = default;

    void Batch::addDrawablesSingleNode(
        const RenderContext& ctx,
        const model::NodeType* type,
        const util::BufferReference instanceRef,
        const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits) noexcept
    {
        const uint32_t drawableCount = instanceRef.size;
        const uint32_t instanceOffset = instanceRef.offset;

        if (drawableCount == 0) return;

        const auto& drawables = m_instanceRegistry->getRange(instanceRef);

        bool frustumChecked = type->m_flags.noFrustum;

        const auto& cameraPos = ctx.m_camera->getWorldPosition();
        const bool forceLineMode = ctx.m_forceLineMode && ctx.m_allowLineMode;
        const bool forceSolid = ctx.m_forceSolid;

        for (uint32_t drawableIndex = 0; drawableIndex < drawables.size(); drawableIndex++) {
            const auto& drawable = drawables[drawableIndex];
            if (drawable.entityIndex == 0) continue;

            const auto& srcDrawOptions = drawable.drawOptions;
            if (!srcDrawOptions.isKind(kindBits)) continue;
            if (srcDrawOptions.m_type == backend::DrawOptions::Type::none) continue;

            auto drawOptions = srcDrawOptions;
            if (forceLineMode) {
                drawOptions.m_lineMode = true;
            }
            if (forceSolid) {
                drawOptions.m_kindBits &= ~render::KIND_BLEND;
            }

            {
                const auto dist2 = glm::distance2(drawable.worldVolume.getCenter(), cameraPos);
                if (drawable.minDistance2 > dist2 ||
	                drawable.maxDistance2 <= dist2)
                {
	                m_skipCount++;
	                return;
                }
            }

            if (!frustumChecked) {
                const auto& frustum = ctx.m_camera->getFrustum();
                // TODO KI wrong volume; assumes that every lodMesh have same
                // => not true in some more complex cases where node consists
                //    from set of meshes (which are not LODn meshes)
                if (m_frustumCPU && !drawable.worldVolume.isOnFrustum(frustum)) {
                    m_skipCount++;
                    return;
                }
                frustumChecked = true;
            }

            auto programId = programSelector(drawable);
            if (!programId) continue;

            programPrepare(programId);

            CommandEntry* commandEntry{ nullptr };
            {
                MultiDrawKey drawKey{
                    programId,
                    drawable.vaoId,
                    drawOptions
                };

                CommandKey commandKey{
                    drawable.baseVertex,
                    drawable.baseIndex,
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
                commandEntry->m_indexCount = drawable.indexCount;

                drawEntry.m_dirty = true;
            }

            commandEntry->addInstance({
                instanceOffset + drawableIndex
                });

            m_drawCount++;
            m_pendingCount++;
        }
    }

    void Batch::addDrawablesInstanced(
        const RenderContext& ctx,
        const model::NodeType* type,
        const util::BufferReference instanceRef,
        const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits) noexcept
    {
        const uint32_t drawableCount = instanceRef.size;
        const uint32_t instanceOffset = instanceRef.offset;

        if (drawableCount == 0) return;

        const auto& drawables = m_instanceRegistry->getRange(instanceRef);

        const auto& cameraPos = ctx.m_camera->getWorldPosition();
        const bool forceLineMode = ctx.m_forceLineMode && ctx.m_allowLineMode;
        const bool forceSolid = ctx.m_forceSolid;

        bool useFrustum = m_frustumCPU;
        {
            if (type->m_flags.noFrustum)
                useFrustum = false;
        }

        {
            if (s_accept.size() < drawableCount) {
                s_accept.resize(drawableCount);
                s_distances2.resize(drawableCount);
            }
            for (uint32_t i = 0; i < drawableCount; i++) {
                s_accept[i] = i; // store index

                const auto& drawable = drawables[i];
                s_distances2[i] = glm::distance2(drawable.worldVolume.getCenter(), cameraPos);
            }
        }

        if (useFrustum) {
            const auto& frustum = ctx.m_camera->getFrustum();

            const auto& checkFrustum = [&frustum, &drawables]
                (int32_t& idx)
                {
                    if (!drawables[idx].worldVolume.isOnFrustum(frustum)) {
                        idx = SKIP;
                    }
                };

            if (drawableCount > m_frustumParallelLimit) {
                std::for_each(
                    std::execution::par_unseq,
                    s_accept.begin(),
                    s_accept.begin() + drawableCount,
                    checkFrustum);
            }
            else {
                std::for_each(
                    std::execution::seq,
                    s_accept.begin(),
                    s_accept.begin() + drawableCount,
                    checkFrustum);
            }
        }

        {
            const auto resolveProgram = [&kindBits, &programSelector](
                const float dist2,
                const auto& drawable) -> ki::program_id {
                if (drawable.minDistance2 > dist2 ||
                    drawable.maxDistance2 <= dist2) return 0;

                const auto& drawOptions = drawable.drawOptions;
                if (drawOptions.m_type == backend::DrawOptions::Type::none) return 0;
                if (!drawOptions.isKind(kindBits)) return 0;

                return programSelector(drawable);
            };

            uint32_t skippedCount = 0;

            for (uint32_t drawableIndex = 0; drawableIndex < drawableCount; drawableIndex++) {
                if (useFrustum && s_accept[drawableIndex] == SKIP) {
                    skippedCount++;
                    continue;
                }

                const auto& drawable = drawables[drawableIndex];
                if (drawable.entityIndex == 0) continue;

                const auto dist2 = s_distances2[drawableIndex];

                const auto  programId = resolveProgram(dist2, drawable);
                if (!programId) continue;

                programPrepare(programId);

                CommandEntry* commandEntry{ nullptr };
                {
                    auto drawOptions = drawable.drawOptions;
                    if (forceLineMode) {
                        drawOptions.m_lineMode = true;
                    }
                    if (forceSolid) {
                        drawOptions.m_kindBits &= ~render::KIND_BLEND;
                    }

                    MultiDrawKey drawKey{
                        programId,
                        drawable.vaoId,
                        drawOptions
                    };

                    CommandKey commandKey{
                        drawable.baseVertex,
                        drawable.baseIndex,
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
                    commandEntry->m_indexCount = drawable.indexCount;

                    drawEntry.m_dirty = true;
                }

                commandEntry->reserve(drawableCount);

                commandEntry->addInstance({
                    instanceOffset + drawableIndex
                    });

                m_pendingCount++;
            }

            m_skipCount += skippedCount;
            m_drawCount += drawableCount - skippedCount;
        }
    }

    void Batch::addMeshes(
        const RenderContext& ctx,
        const util::BufferReference instanceRef,
        uint8_t kindBits) noexcept
    {
        const uint32_t drawableCount = instanceRef.size;
        const uint32_t instanceOffset = instanceRef.offset;

        if (drawableCount == 0) return;

        const auto& drawables = m_instanceRegistry->getRange(instanceRef);

        const auto& cameraPos = ctx.m_camera->getWorldPosition();
        const bool forceLineMode = ctx.m_forceLineMode && ctx.m_allowLineMode;
        const bool forceSolid = ctx.m_forceSolid;

        bool useFrustum = m_frustumCPU;

        {
            if (s_accept.size() < drawableCount) {
                s_accept.resize(drawableCount);
                s_distances2.resize(drawableCount);
            }
            for (uint32_t i = 0; i < drawableCount; i++) {
                s_accept[i] = i; // store index

                const auto& drawable = drawables[i];
                s_distances2[i] = glm::distance2(drawable.worldVolume.getCenter(), cameraPos);
            }
        }

        if (useFrustum) {
            const auto& frustum = ctx.m_camera->getFrustum();

            const auto& checkFrustum = [&frustum, &drawables]
            (int32_t& idx) {
                if (!drawables[idx].worldVolume.isOnFrustum(frustum)) {
                    idx = SKIP;
                }
            };

            if (drawableCount > m_frustumParallelLimit) {
                std::for_each(
                    std::execution::par_unseq,
                    s_accept.begin(),
                    s_accept.begin() + drawableCount,
                    checkFrustum);
            }
            else {
                std::for_each(
                    std::execution::seq,
                    s_accept.begin(),
                    s_accept.begin() + drawableCount,
                    checkFrustum);
            }
        }

        {
            const auto resolveProgram = [&kindBits](
                const float dist2,
                const auto& drawable) -> ki::program_id {
                if (drawable.minDistance2 > dist2 ||
                    drawable.maxDistance2 <= dist2) return 0;

                const auto& drawOptions = drawable.drawOptions;
                if (drawOptions.m_type == backend::DrawOptions::Type::none) return 0;
                if (!drawOptions.isKind(kindBits)) return 0;

                return drawable.programId;
            };

            uint32_t skippedCount = 0;

            for (uint32_t drawableIndex = 0; drawableIndex < drawableCount; drawableIndex++) {
                if (useFrustum && s_accept[drawableIndex] == SKIP) {
                    skippedCount++;
                    continue;
                }

                const auto& drawable = drawables[drawableIndex];
                if (drawable.entityIndex == 0) continue;

                const auto dist2 = s_distances2[drawableIndex];

                const auto  programId = resolveProgram(dist2, drawable);
                if (!programId) continue;

                CommandEntry* commandEntry{ nullptr };
                {
                    auto drawOptions = drawable.drawOptions;
                    if (forceLineMode) {
                        drawOptions.m_lineMode = true;
                    }
                    if (forceSolid) {
                        drawOptions.m_kindBits &= ~render::KIND_BLEND;
                    }

                    MultiDrawKey drawKey{
                        programId,
                        drawable.vaoId,
                        drawOptions
                    };

                    CommandKey commandKey{
                        drawable.baseVertex,
                        drawable.baseIndex,
                    };

                    const auto drawIndex = m_batchRegistry.getMultiDrawIndex(drawKey);
                    const auto commandIndex = m_batchRegistry.getCommandIndex(commandKey);

                    if (m_pending.size() < drawIndex + 1) {
                        m_pending.resize(drawIndex + 1);
                    }

                    auto& drawEntry = m_pending[drawIndex];
                    drawEntry.m_index = drawIndex;

                    if (drawEntry.m_commands.size() < commandIndex + 1) {
                        drawEntry.m_commands.resize(commandIndex + 1);
                    }

                    commandEntry = &drawEntry.m_commands[commandIndex];
                    commandEntry->m_index = commandIndex;
                    commandEntry->m_indexCount = drawable.indexCount;

                    drawEntry.m_dirty = true;
                }

                commandEntry->reserve(drawableCount);

                commandEntry->addInstance({
                    instanceOffset + drawableIndex
                    });

                m_pendingCount++;
            }

            m_skipCount += skippedCount;
            m_drawCount += drawableCount - skippedCount;
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

        const auto& assets = ctx.getAssets();

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

        m_draw = std::make_unique<backend::DrawBuffer>();
        m_draw->prepareRT();

        m_frustumCPU = assets.frustumEnabled && assets.frustumCPU;
        m_frustumGPU = assets.frustumEnabled && assets.frustumGPU;
        m_frustumParallelLimit = assets.frustumParallelLimit;

        m_instanceRegistry = &render::InstanceRegistry::get();
    }

    void Batch::updateRT(
        const UpdateContext& ctx)
    {
        const auto& assets = ctx.getAssets();
        const auto& dbg = ctx.getDebug();

        m_frustumCPU = assets.frustumEnabled && assets.frustumCPU && dbg.m_frustumEnabled;

        m_batchRegistry.optimizeMultiDrawOrder();
        m_pending.resize(m_batchRegistry.getMaxMultDrawIndex() + 1);
    }

    void Batch::beginFrame()
    {
        m_frameFlushCount = 0;
        m_draw->beginFrame();
    }

    void Batch::endFrame()
    {
        //KI_INFO(fmt::format("BATCH: frame_batch_flushes={}", m_frameFlushCount));
        m_frameFlushCount = 0;
        m_draw->endFrame();
    }

    void Batch::draw(
        const RenderContext& ctx,
        model::Node* node,
        const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits)
    {
        if (node->m_typeFlags.invisible || !node->m_visible || !node->m_alive) return;

        node->updateVAO(ctx);
        node->bindBatch(ctx, programSelector, programPrepare, kindBits, *this);
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
        m_instanceIndeces.clear();
        m_pendingCount = 0;
    }

    size_t Batch::flush(
        const RenderContext& ctx)
    {
        if (m_pendingCount == 0) return 0;

        size_t flushCount = 0;

        // Setup instances
        {
            m_instanceIndeces.clear();

            for (auto& multiDraw : m_pending)
            {
                if (multiDraw.empty()) continue;

                const auto& multiDrawKey = *m_batchRegistry.getMultiDraw(multiDraw.m_index);

                for (auto& command : multiDraw.m_commands)
                {
                    if (command.empty()) continue;

                    command.m_baseIndex = static_cast<uint32_t>(m_instanceIndeces.size());

                    for (uint32_t i = 0; i < command.m_instanceCount; i++)
                    {
                        const auto& lodEntry = command.m_instances[i];
                        m_instanceIndeces.emplace_back(lodEntry.m_instanceIndex);
                    }
                }
            }

            if (m_instanceIndeces.empty()) {
                clearBatches();
                return 0;
            }
        }

        flushCount = m_instanceIndeces.size();
        m_flushedTotalCount += flushCount;

        // NOTE KI baseVertex usage
        // https://community.khronos.org/t/vertex-buffer-management-with-indirect-drawing/77272
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Instanced_arrays
        //

        auto* draw = m_draw.get();

        draw->sendInstanceIndeces(m_instanceIndeces);

        backend::gl::DrawIndirectCommand indirect{};

        for (const auto& multiDraw : m_pending) {
            if (multiDraw.empty()) continue;

            const auto& multiDrawKey = *m_batchRegistry.getMultiDraw(multiDraw.m_index);

            backend::MultiDrawRange drawRange = {
                multiDrawKey.m_drawOptions,
                multiDrawKey.m_vaoId,
                multiDrawKey.m_programId,
            };

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
            clearBatches();
        }

        m_frameFlushCount++;

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
