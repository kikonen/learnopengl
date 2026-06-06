#include "Batch.h"
#include "Batch.h"

#include <mutex>
#include <iostream>
#include <algorithm>
#include <execution>
#include <cassert>

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
}

namespace render {
    Batch::Batch()
    {
    }

    Batch::~Batch() = default;

    void Batch::addDrawablesSingleNode(
        const RenderContext& ctx,
        const util::BufferReference instanceRef,
        const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits) noexcept
    {
        addDrawablesImpl(ctx, instanceRef, programSelector, programPrepare, kindBits);
    }

    void Batch::addDrawablesInstanced(
        const RenderContext& ctx,
        const util::BufferReference instanceRef,
        const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits) noexcept
    {
        addDrawablesImpl(ctx, instanceRef, programSelector, programPrepare, kindBits);
    }

    void Batch::addDrawablesImpl(
        const RenderContext& ctx,
        const util::BufferReference instanceRef,
        const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits) noexcept
    {
        const uint32_t drawableCount = instanceRef.size;
        const uint32_t instanceOffset = instanceRef.offset;

        if (drawableCount == 0) return;

        const auto& drawables = m_instanceRegistry->getRange(instanceRef);

        // NOTE KI frustum + LOD visibility (incl. per-drawable noFrustum) is
        // precomputed once per camera in InstanceRegistry::cullFrustum; here we
        // only read the cached flags. A drawable renders only when all bits are set.
        const auto& visible = m_instanceRegistry->getVisibleRange(instanceRef);
        const bool haveVis = !visible.empty();
        assert(!(m_frustumCPU || m_lodDistanceEnabled) ||
            m_instanceRegistry->cullSignatureMatches(ctx.m_camera->getFrustum()));

        for (uint32_t drawableIndex = 0; drawableIndex < drawables.size(); drawableIndex++) {
            const auto& drawable = drawables[drawableIndex];
            if (drawable.entityIndex == 0) continue;

            const auto& drawOptions = drawable.drawOptions;
            if (drawOptions.m_type == backend::DrawOptions::Type::none) continue;
            if (!drawOptions.isKind(kindBits)) continue;

            if (haveVis && (visible[drawableIndex] & VISIBLE_ALL) != VISIBLE_ALL) {
                m_skipCount++;
                continue;
            }

            const auto programId = programSelector(drawable);
            if (!programId) continue;

            programPrepare(programId);

            CommandEntry* commandEntry{ nullptr };
            {
                MultiDrawEntry* drawEntry;
                {
                    MultiDrawKey drawKey{
                        programId,
                        drawable.vaoId,
                        drawOptions
                    };

                    const auto drawIndex = m_batchRegistry.getMultiDrawIndex(drawKey);
                    drawEntry = m_drawEntryContainer.addDrawEntry(drawIndex);
                }
                {
                    CommandKey commandKey{
                        drawable.baseVertex,
                        drawable.baseIndex,
                    };
                    const auto commandIndex = m_batchRegistry.getCommandIndex(commandKey);
                    commandEntry = drawEntry->addCommandEntry(commandIndex, drawable.indexCount);
                }
            }

            // NOTE KI hint for the instanced case (many instances share one command);
            // harmless for the single-node case
            commandEntry->reserve(drawableCount);

            commandEntry->addInstance({
                instanceOffset + drawableIndex
                });

            m_drawCount++;
            m_pendingCount++;
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

        // NOTE KI frustum + LOD visibility precomputed once per camera (cullFrustum)
        const auto& visible = m_instanceRegistry->getVisibleRange(instanceRef);
        const bool haveVis = !visible.empty();
        assert(!(m_frustumCPU || m_lodDistanceEnabled) ||
            m_instanceRegistry->cullSignatureMatches(ctx.m_camera->getFrustum()));

        {
            const auto resolveProgram = [&kindBits, this](
                const auto& drawable) -> ki::program_id
            {
                const auto& drawOptions = drawable.drawOptions;
                if (drawOptions.m_type == backend::DrawOptions::Type::none) return 0;
                if (!drawOptions.isKind(kindBits)) return 0;

                return drawable.programId;
            };

            uint32_t skippedCount = 0;

            for (uint32_t drawableIndex = 0; drawableIndex < drawableCount; drawableIndex++) {
                if (haveVis && (visible[drawableIndex] & VISIBLE_ALL) != VISIBLE_ALL) {
                    skippedCount++;
                    continue;
                }

                const auto& drawable = drawables[drawableIndex];
                if (drawable.entityIndex == 0) continue;

                const auto  programId = resolveProgram(drawable);
                if (!programId) continue;

                CommandEntry* commandEntry{ nullptr };
                {
                    const auto drawOptions = drawable.drawOptions;

                    MultiDrawEntry* drawEntry;
                    {
                        MultiDrawKey drawKey{
                            programId,
                            drawable.vaoId,
                            drawOptions
                        };

                        const auto drawIndex = m_batchRegistry.getMultiDrawIndex(drawKey);
                        drawEntry = m_drawEntryContainer.addDrawEntry(drawIndex);
                    }
                    {
                        CommandKey commandKey{
                            drawable.baseVertex,
                            drawable.baseIndex,
                        };
                        const auto commandIndex = m_batchRegistry.getCommandIndex(commandKey);
                        commandEntry = drawEntry->addCommandEntry(commandIndex, drawable.indexCount);
                    }
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

        const auto& dbg = ctx.getDebug();

        m_frustumCPU = dbg.m_frustumEnabled && assets.frustumCPU;
        m_frustumGPU = dbg.m_frustumEnabled && assets.frustumGPU;
        m_lodDistanceEnabled = dbg.m_lodDistanceEnabled;
        m_frustumParallelLimit = assets.frustumParallelLimit;

        m_instanceRegistry = &render::InstanceRegistry::get();
    }

    void Batch::updateRT(
        const UpdateContext& ctx)
    {
        const auto& assets = ctx.getAssets();
        const auto& dbg = ctx.getDebug();

        m_frustumCPU = dbg.m_frustumEnabled && assets.frustumCPU;
        m_frustumGPU = dbg.m_frustumEnabled && assets.frustumGPU;
        m_lodDistanceEnabled = dbg.m_lodDistanceEnabled;

        //m_batchRegistry.optimizeMultiDrawOrder();
    }

    void Batch::beginFrame()
    {
        m_frameFlushCount = 0;
        m_draw->beginFrame();
        clearBatches();
    }

    void Batch::endFrame()
    {
        //KI_INFO(fmt::format("BATCH: frame_batch_flushes={}", m_frameFlushCount));
        m_frameFlushCount = 0;
        m_draw->endFrame();
        clearBatches();
    }

    void Batch::cullFrustum(const RenderContext& ctx)
    {
        m_instanceRegistry->cullFrustum(
            ctx.m_camera->getFrustum(),
            ctx.m_camera->getWorldPosition(),
            m_frustumCPU,
            m_lodDistanceEnabled,
            m_frustumParallelLimit);
    }

    void Batch::draw(
        const RenderContext& ctx,
        model::Node* node,
        const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits)
    {
        if (node->m_typeFlags.invisible || !node->m_visible || !node->m_alive) return;
        node->addToBatch(ctx, programSelector, programPrepare, kindBits, *this);
    }

    bool Batch::isFlushed() const noexcept
    {
        return m_pendingCount == 0;
    }

    void Batch::clearBatches() noexcept
    {
        //m_batchRegistry.clear();
        m_drawEntryContainer.clear();
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

            for (auto& multiDraw : m_drawEntryContainer.m_pending)
            {
                if (multiDraw.empty()) continue;

                //const auto& multiDrawKey = *m_batchRegistry.getMultiDraw(multiDraw.m_index);

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

        const bool forceLineMode = ctx.m_forceLineMode && ctx.m_allowLineMode;
        const bool forceSolid = ctx.m_forceSolid;

        backend::gl::DrawIndirectCommand indirect{};

        for (const auto& multiDraw : m_drawEntryContainer.m_pending) {
            if (multiDraw.empty()) continue;

            const auto& multiDrawKey = *m_batchRegistry.getMultiDraw(multiDraw.m_index);

            backend::MultiDrawRange drawRange = {
                multiDrawKey.m_drawOptions,
                multiDrawKey.m_vaoId,
                multiDrawKey.m_programId,
                forceLineMode,
                forceSolid
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

        //// In Batch::flush, after line 579 (flushCount = m_instanceIndeces.size();)
        //KI_INFO_OUT(fmt::format("BATCH_FLUSH: instances={}, multiDraws={}",
        //    flushCount, m_drawEntryContainer.m_pending.size()));

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
