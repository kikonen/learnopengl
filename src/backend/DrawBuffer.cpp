#include "DrawBuffer.h"

#include <fmt/format.h>

#include "kigl/GLState.h"

#include "util/Util.h"

#include "asset/Assets.h"
#include "asset/SSBO.h"

#include "asset/Program.h"
#include "asset/ProgramUniforms.h"
#include "asset/Shader.h"
#include "asset/uniform.h"

#include "kigl/GLSyncQueue_impl.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"

#include "backend/gl/DrawIndirectParameters.h"
#include "backend/gl/PerformanceCounters.h"
#include "DrawOptions.h"

namespace {
    constexpr size_t INDEX_BLOCK_SIZE = 1000;
    constexpr size_t INDEX_BLOCK_COUNT = 500;

    constexpr size_t MAX_INDEX_COUNT = INDEX_BLOCK_SIZE * INDEX_BLOCK_COUNT;
}

namespace backend {
    constexpr int BUFFER_ALIGNMENT = 1;

    DrawBuffer::DrawBuffer(
        bool useMapped,
        bool useInvalidate,
        bool useFence,
        bool useSingleFence,
        bool useDebugFence)
        : m_useMapped{ useMapped },
        m_useInvalidate{ useInvalidate },
        m_useFence(useFence),
        m_useSingleFence(useSingleFence),
        m_useDebugFence{ useDebugFence }
    {
    }

    void DrawBuffer::prepareRT(
        const PrepareContext& ctx,
        int batchCount,
        int rangeCount)
    {
        const auto info = kigl::GL::getInfo();

        const auto& assets = Assets::get();
        auto& registry = ctx.m_registry;

        m_frustumGPU = assets.frustumEnabled && assets.frustumGPU;

        m_batchCount = batchCount;
        m_rangeCount = rangeCount;

        int batchMultiplier = 1;

        //batchMultiplier = rangeCount;
        //rangeCount = 1;

        int commandBatchCount = batchCount * batchMultiplier;
        int commandRangeCount = rangeCount;

        if (m_frustumGPU) {
            m_computeGroups = assets.computeGroups;
            m_cullingCompute = ProgramRegistry::get().getComputeProgram(
                CS_FRUSTUM_CULLING,
                {
                    { DEF_FRUSTUM_DEBUG, std::to_string(assets.frustumDebug) },
                    { DEF_CS_GROUP_X, std::to_string(m_computeGroups[0])},
                    { DEF_CS_GROUP_Y, std::to_string(m_computeGroups[1]) },
                    { DEF_CS_GROUP_Z, std::to_string(m_computeGroups[2]) },
                });

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

            auto* data = (gl::PerformanceCounters*)m_performanceCounters.m_data;
            *data = { 0, 0 };
        }

        m_commands = std::make_unique<GLCommandQueue>(
            "draw_command",
            commandBatchCount,
            commandRangeCount,
            m_useMapped,
            m_useInvalidate,
            m_useFence,
            m_useSingleFence,
            m_useDebugFence);
        m_commands->prepare(BUFFER_ALIGNMENT, assets.batchDebug);

        m_drawRanges.reserve(rangeCount);
        for (int i = 0; i < rangeCount; i++) {
            m_drawRanges.emplace_back();
        }

        //m_indexBuffer.createEmpty(INDEX_BLOCK_SIZE * sizeof(GLuint), GL_DYNAMIC_STORAGE_BIT);
        m_indexBuffer.createEmpty(1 * sizeof(GLuint), GL_DYNAMIC_STORAGE_BIT);
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
        const auto& drawRange = m_drawRanges[cmdRange.m_index];
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
            auto* data = (gl::DrawIndirectParameters*)m_drawParameters.m_data;
            data += cmdRange.m_index;

            data->u_baseIndex = static_cast<GLuint>(cmdRange.m_baseIndex);
            data->u_drawType = static_cast<GLuint>(util::as_integer(drawRange.m_drawOptions.m_type));
            data->u_drawCount = static_cast<GLuint>(drawCount);

            constexpr size_t PARAMS_SZ = sizeof(gl::DrawIndirectParameters);
            m_drawParameters.flushRange(
                cmdRange.m_index * PARAMS_SZ,
                PARAMS_SZ);
        }

        if (m_frustumGPU) {
            m_cullingCompute->bind();

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

            glDispatchCompute(drawCount, 1, 1);
        }

        m_drawCounter += drawCount;

        const auto& next = m_commands->next();
        if (!next.empty()) {
            // NOTE KI trigger draw pending if out of buffers
            drawPending(true);
        }
    }

    void DrawBuffer::flushIfNotSame(
        const backend::DrawRange& sendRange)
    {
        const auto& cmdRange = m_commands->current();
        auto& curr = m_drawRanges[cmdRange.m_index];

        bool sameDraw = true;
        if (!cmdRange.empty()) {
            sameDraw = curr.m_program == sendRange.m_program &&
                curr.m_vao == sendRange.m_vao &&
                curr.m_drawOptions.isSameMultiDraw(
                    sendRange.m_drawOptions,
                    curr.m_forceWireframe,
                    curr.m_allowBlend);
        }

        if (!sameDraw) {
            flush();
        }
    }

    void DrawBuffer::send(
        const backend::DrawRange& sendRange,
        const backend::gl::DrawIndirectCommand& cmd)
    {
        flushIfNotSame(sendRange);

        const auto& cmdRange = m_commands->current();
        auto& curr = m_drawRanges[cmdRange.m_index];

        if (cmdRange.empty()) {
            // starting new range
            curr = sendRange;
        }

        if (m_commands->send(cmd)) {
            flush();
        }
    }

    void DrawBuffer::sendInstanceIndeces(
        std::span<GLuint> indeces)
    {
        {
            const size_t totalCount = indeces.size();
            constexpr size_t sz = sizeof(GLuint);

            // NOTE KI *reallocate* SSBO if needed
            if (m_indexBuffer.m_size < totalCount * sz) {
                size_t blocks = (totalCount / INDEX_BLOCK_SIZE) + 2;
                size_t bufferSize = blocks * INDEX_BLOCK_SIZE * sz;
                m_indexBuffer.resizeBuffer(bufferSize);
            }

            m_indexBuffer.invalidateRange(
                0 * sz,
                totalCount * sz);

            m_indexBuffer.update(
                0 * sz,
                totalCount * sz,
                indeces.data());
        }

        m_indexBuffer.bindSSBO(SSBO_INSTANCE_INDECES);
    }

    void DrawBuffer::drawPending(bool drawCurrent)
    {
        if (m_frustumGPU) {
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
        }

        size_t count = 0;
        auto handler = [this, &count](kigl::GLBufferRange& cmdRange) {
            auto& drawRange = m_drawRanges[cmdRange.m_index];
            const auto& drawOptions = drawRange.m_drawOptions;
            const GLsizei drawCount = (GLsizei)cmdRange.m_usedCount;

            bindDrawRange(drawRange);

            if (drawOptions.m_type == backend::DrawOptions::Type::elements) {
                glMultiDrawElementsIndirect(
                    drawOptions.m_mode,
                    GL_UNSIGNED_INT,
                    (void*)cmdRange.m_baseOffset,
                    drawCount,
                    sizeof(backend::gl::DrawIndirectCommand));
            }
            else if (drawOptions.m_type == backend::DrawOptions::Type::arrays)
            {
                glMultiDrawArraysIndirect(
                    drawOptions.m_mode,
                    (void*)cmdRange.m_baseOffset,
                    drawCount,
                    sizeof(backend::gl::DrawIndirectCommand));
            }
            count += drawCount;

            // NOTE KI need to wait finishing of draw commands
            // TODO KI *very* odd that fence was set but not waited anywhere
            //if (m_useFence) {
            //    cmdRange.setFence();
            //}
        };

        m_commands->processPending(handler, drawCurrent, true);
    }

    gl::PerformanceCounters DrawBuffer::getCounters(bool clear) const
    {
        gl::PerformanceCounters counters;

        if (m_frustumGPU) {
            auto* data = (gl::PerformanceCounters*)m_performanceCounters.m_data;
            counters = *data;

            if (clear) {
                *data = { 0, 0 };
            }
        }

        return counters;
    }

    void DrawBuffer::bindDrawRange(
        const backend::DrawRange& drawRange) const
    {
        const auto& drawOptions = drawRange.m_drawOptions;
        auto& state = kigl::GLState::get();

        drawRange.m_program->bind();
        state.bindVAO(*drawRange.m_vao);

        state.setEnabled(GL_CULL_FACE, !drawOptions.m_renderBack);

        const bool wireframe = drawOptions.m_wireframe || drawRange.m_forceWireframe;

        if (wireframe) {
            state.polygonFrontAndBack(GL_LINE);
        }
        else {
            state.polygonFrontAndBack(GL_FILL);
        }

        if (drawOptions.m_tessellation) {
            glPatchParameteri(GL_PATCH_VERTICES, drawOptions.m_patchVertices);
        }

        const bool blend = !wireframe && (drawOptions.m_blend || drawOptions.m_blendOIT) && drawRange.m_allowBlend;
        state.setEnabled(GL_BLEND, blend);
        if (blend) {
            if (!drawOptions.m_blendOIT) {
                state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });
            }
        }
    }
}
