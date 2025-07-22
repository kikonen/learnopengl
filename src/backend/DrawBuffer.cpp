#include "DrawBuffer.h"

#include <tuple>

#include <fmt/format.h>

#include "kigl/GLState.h"

#include "util/util.h"

#include "asset/Assets.h"

#include "shader/SSBO.h"
#include "shader/Program.h"
#include "shader/ProgramUniforms.h"
#include "shader/Shader.h"
#include "shader/uniform.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLSyncQueue_impl.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"

#include "backend/gl/DrawIndirectParameters.h"
#include "backend/gl/PerformanceCounters.h"
#include "DrawOptions.h"

namespace {
    inline const std::string CS_FRUSTUM_CULLING{ "frustum_culling" };

    constexpr size_t INDEX_BLOCK_SIZE = 1000;
    constexpr size_t INDEX_BLOCK_COUNT = 500;

    constexpr size_t MAX_INDEX_COUNT = INDEX_BLOCK_SIZE * INDEX_BLOCK_COUNT;

    constexpr size_t MAX_INSTANCE_BUFFERS = 256;
}

namespace backend {
    constexpr int BUFFER_ALIGNMENT = 1;

    DrawBuffer::DrawBuffer(
        bool useMapped,
        bool useInvalidate,
        bool useFence,
        bool useFenceDebug)
        : m_useMapped{ useMapped },
        m_useInvalidate{ useInvalidate },
        m_useFence(useFence),
        m_useFenceDebug{ useFenceDebug }
    {
    }

    void DrawBuffer::prepareRT(
        const PrepareContext& ctx,
        int batchCount,
        int rangeCount)
    {
        const auto info = kigl::GL::getInfo();

        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;

        m_frustumGPU = assets.frustumEnabled && assets.frustumGPU;

        m_batchDebug = assets.batchDebug;
        m_batchCount = batchCount;
        m_rangeCount = rangeCount;

        int batchMultiplier = 1;

        //batchMultiplier = rangeCount;
        //rangeCount = 1;

        int commandBatchCount = batchCount * batchMultiplier;
        int commandRangeCount = rangeCount;

        if (m_frustumGPU) {
            m_computeGroups = assets.computeGroups;
            m_cullingComputeId = ProgramRegistry::get().getComputeProgram(
                CS_FRUSTUM_CULLING,
                {
                    { DEF_FRUSTUM_DEBUG, std::to_string(assets.frustumDebug) },
                    { DEF_CS_GROUP_X, std::to_string(m_computeGroups[0])},
                    { DEF_CS_GROUP_Y, std::to_string(m_computeGroups[1]) },
                    { DEF_CS_GROUP_Z, std::to_string(m_computeGroups[2]) },
                });
            m_cullingCompute = Program::get(m_cullingComputeId);
            m_cullingCompute->prepareRT();
        }

        if (m_frustumGPU) {
            constexpr int storageFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT;
            constexpr int mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;

            m_drawParameters.createEmpty(rangeCount * sizeof(gl::DrawIndirectParameters), storageFlags);
            m_drawParameters.map(mapFlags);
        }

        if (m_frustumGPU) {
            constexpr int storageFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            constexpr int mapFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

            m_performanceCounters.createEmpty(rangeCount * sizeof(gl::PerformanceCounters), storageFlags);
            m_performanceCounters.map(mapFlags);

            auto* mappedData = m_performanceCounters.mapped<gl::PerformanceCounters>(0);
            *mappedData = { 0, 0 };
        }

        m_commands = std::make_unique<GLCommandQueue>(
            "draw_command",
            commandBatchCount,
            commandRangeCount,
            m_useMapped,
            m_useInvalidate,
            m_useFence,
            m_useFenceDebug);
        m_commands->prepare(BUFFER_ALIGNMENT, m_batchDebug);
    }

    void DrawBuffer::bind()
    {
        if (m_bound) return;
        m_bound = true;

        if (m_frustumGPU) {
            m_drawParameters.bindParameter();
            m_drawParameters.bindSSBO(SSBO_DRAW_PARAMETERS);
        }

        if (m_frustumGPU) {
            m_performanceCounters.bindSSBO(SSBO_PERFORMANCE_COUNTERS);
        }

        {
            m_commands->m_buffer.bindDrawIndirect();
        }
        if (m_frustumGPU) {
            m_commands->m_buffer.bindSSBO(SSBO_DRAW_COMMANDS);
        }
    }

    void DrawBuffer::flushIfNeeded()
    {
        if (!m_commands->full()) return;
        flush();
    }

    void DrawBuffer::flush()
    {
        const auto& cmdRange = m_commands->current();
        const int drawCount = static_cast<int>(cmdRange.m_usedCount);

        if (drawCount == 0) return;

        m_commands->flush();

        // NOTE KI
        // - bind CS program
        // - execute CS program
        // - glMemoryBarrier
        // - bind draw program
        // - execute draw program

        if (m_frustumGPU) {
            auto* data = m_drawParameters.mapped< gl::DrawIndirectParameters>(0);
            data += cmdRange.m_index;

            data->u_baseIndex = static_cast<GLuint>(cmdRange.m_baseIndex);
            // TODO KI broken
            //data->u_drawType = static_cast<GLuint>(util::as_integer(drawRange.m_drawOptions.m_type));
            data->u_drawCount = static_cast<GLuint>(drawCount);

            constexpr size_t PARAMS_SZ = sizeof(gl::DrawIndirectParameters);
            m_drawParameters.flushRange(
                cmdRange.m_index * PARAMS_SZ,
                PARAMS_SZ);
        }

        if (m_frustumGPU) {
            m_cullingCompute->m_uniforms->u_drawParametersIndex.set(static_cast<GLuint>(cmdRange.m_index));

            //const int maxX = m_computeGroups[0];
            //int groupX = drawCount;
            //int groupY = 1;
            //if (drawCount > maxX) {
            //    groupX = maxX;
            //    groupY = drawCount / maxX;
            //    if (drawCount % maxX != 0) groupY++;
            //}
            //glDispatchCompute(m_computeGroups[0], groupY, 1);

            m_cullingCompute->bind();
            glDispatchCompute(drawCount, 1, 1);
        }

        m_drawCounter += drawCount;

        drawPending();
        m_commands->next();
    }

    void DrawBuffer::finish()
    {
        // NOTE KI instances are fenced after their associated draw
        m_instanceBuffers->setFence();
        m_instanceBuffers->next();
    }

    bool DrawBuffer::isSameMultiDraw(
        const backend::MultiDrawRange& sendRange)
    {
        const auto& cmdRange = m_commands->current();
        if (cmdRange.empty()) return false;

        auto& curr = m_drawRanges.back().second;

        const auto& cd = curr.m_drawOptions;
        const auto& sd = sendRange.m_drawOptions;

        // NOTE KI KIND_SOLID & KIND_ALPHA can be in same multidraw
        return curr.m_vaoId == sendRange.m_vaoId &&
            curr.m_programId == sendRange.m_programId &&
            cd.m_renderBack == sd.m_renderBack &&
            cd.m_lineMode == sd.m_lineMode &&
            cd.isBlend() == sd.isBlend() &&
            cd.m_mode == sd.m_mode &&
            cd.m_type == sd.m_type;
    }

    void DrawBuffer::send(
        const backend::MultiDrawRange& sendRange,
        const backend::gl::DrawIndirectCommand& cmd)
    {
        const auto& cmdRange = m_commands->current();

        if (isSameMultiDraw(sendRange)) {
            m_drawRanges.back().first++;
        }
        else {
            // starting new range
            m_drawRanges.push_back({ 1, sendRange });
        }

        if (m_commands->send(cmd)) {
            KI_INFO_OUT("full: flush");
            flush();
        }
    }

    void DrawBuffer::sendInstanceIndeces(
        std::span<mesh::InstanceSSBO> indeces)
    {
        const size_t totalCount = indeces.size();
        {
            createInstanceBuffers(totalCount);

            auto& current = m_instanceBuffers->current();
            auto* mappedData = m_instanceBuffers->currentMapped();

            m_instanceBuffers->waitFence();
            std::copy(
                std::begin(indeces),
                std::end(indeces),
                mappedData);
            m_instanceBuffers->bindCurrentSSBO(SSBO_INSTANCE_INDECES, false, totalCount);

            // NOTE KI instances are fenced after their associated draw
        }
    }

    void DrawBuffer::createInstanceBuffers(size_t totalCount)
    {
        if (!m_instanceBuffers || m_instanceBuffers->getEntryCount() < totalCount) {
            size_t blocks = (totalCount / INDEX_BLOCK_SIZE) + 2;
            size_t entryCount = blocks * INDEX_BLOCK_SIZE;

            m_instanceBuffers = std::make_unique<kigl::GLSyncQueue<mesh::InstanceSSBO>>(
                "draw_instance",
                entryCount,
                MAX_INSTANCE_BUFFERS,
                true,
                false,
                true,
                m_useFenceDebug);
            m_instanceBuffers->prepare(BUFFER_ALIGNMENT, m_batchDebug);
        }
    }

    void DrawBuffer::drawPending()
    {
        if (m_frustumGPU) {
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
        }

        size_t tallyCount = 0;
        auto handler = [this, &tallyCount](kigl::GLBufferRange& cmdRange) {
            constexpr auto sz = sizeof(backend::gl::DrawIndirectCommand);

            for (const auto& drawEntry : m_drawRanges) {
                const auto& drawRange = drawEntry.second;
                const auto& drawCount = drawEntry.first;
                const auto& baseOffset = cmdRange.m_baseOffset + tallyCount * sz;

                const auto& drawOptions = drawRange.m_drawOptions;

                bindMultiDrawRange(drawRange);

                if (drawOptions.m_type == backend::DrawOptions::Type::elements) {
                    glMultiDrawElementsIndirect(
                        drawOptions.toMode(),
                        GL_UNSIGNED_INT,
                        (void*)baseOffset,
                        drawCount,
                        sz);
                }
                else if (drawOptions.m_type == backend::DrawOptions::Type::arrays)
                {
                    glMultiDrawArraysIndirect(
                        drawOptions.toMode(),
                        (void*)baseOffset,
                        drawCount,
                        sz);
                }
                tallyCount += drawCount;
            }

            assert(cmdRange.m_usedCount == tallyCount);
            if (cmdRange.m_usedCount != tallyCount) throw "CORRUPTED";
        };

        m_commands->processCurrent(handler);
        m_drawRanges.clear();
    }

    gl::PerformanceCounters DrawBuffer::getCounters(bool clear) const
    {
        gl::PerformanceCounters counters;

        if (m_frustumGPU) {
            auto* mappedData = m_performanceCounters.mapped<gl::PerformanceCounters>(0);
            counters = *mappedData;

            if (clear) {
                *mappedData = { 0, 0 };
            }
        }

        return counters;
    }

    void DrawBuffer::bindMultiDrawRange(
        const backend::MultiDrawRange& drawRange) const
    {
        auto& state = kigl::GLState::get();
        const auto& drawOptions = drawRange.m_drawOptions;

        // NOTE KI bind vao only if used for this draw
        if (drawRange.m_vaoId) {
            state.bindVAO(drawRange.m_vaoId);
        }

        auto* program = Program::get(drawRange.m_programId);
        state.useProgram(*program);

        state.setEnabled(GL_CULL_FACE, !drawOptions.m_renderBack);

        const bool lineMode = drawOptions.m_lineMode;

        if (drawOptions.m_reverseFrontFace) {
            state.frontFace(GL_CW);
        }
        else {
            state.frontFace(GL_CCW);
        }

        if (drawOptions.m_noDepth) {
            state.setEnabled(GL_DEPTH_TEST, false);
        }
        else {
            state.setEnabled(GL_DEPTH_TEST, true);
        }

        state.polygonFrontAndBack(lineMode ? GL_LINE : GL_FILL);

        if (drawOptions.m_mode == DrawOptions::Mode::patches) {
            glPatchParameteri(GL_PATCH_VERTICES, drawOptions.m_patchVertices);
        }

        const bool blend = !lineMode && drawOptions.isBlend();
        state.setEnabled(GL_BLEND, blend);
        if (blend) {
            // NOTE KI no blend mode with OIT blend
            if (!drawOptions.m_gbuffer) {
                state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });
            }
        }

        state.setEnabled(GL_CLIP_DISTANCE1, drawRange.m_drawOptions.m_clip);

        // HACK KI for primitive GL_LINES
        // OPENGL: API 0x7 (7) DEPRECATED MEDIUM - API_ID_LINE_WIDTH
        // deprecated behavior warning has been generated.
        // Wide lines have been deprecated. glLineWidth set to 1.500000.
        // glLineWidth with width greater than 1.0 will generate GL_INVALID_VALUE
        // error in future versions
        //glLineWidth(1.5f);
    }
}
