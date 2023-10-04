#include "DrawBuffer.h"

#include <fmt/format.h>

#include "asset/Assets.h"
#include "asset/SSBO.h"
#include "asset/Program.h"
#include "asset/Shader.h"
#include "asset/uniform.h"

#include "util/Util.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"

#include "backend/gl/DrawIndirectParameters.h"
#include "backend/gl/PerformanceCounters.h"
#include "DrawOptions.h"


namespace backend {
    constexpr int BUFFER_ALIGNMENT = 1;

    DrawBuffer::DrawBuffer(
        bool useFence,
        bool useSingleFence)
        : m_useFence(useFence),
        m_useSingleFence(useSingleFence)
    {
    }

    void DrawBuffer::prepare(
        const Assets& assets,
        Registry* registry,
        int batchCount,
        int rangeCount)
    {
        const auto info = kigl::GL::getInfo();

        m_frustumGPU = assets.frustumEnabled && assets.frustumGPU;

        m_batchCount = batchCount;
        m_rangeCount = rangeCount;

        int batchMultiplier = 1;

        //batchMultiplier = rangeCount;
        //rangeCount = 1;

        int commandBatchCount = batchCount * batchMultiplier;
        int commandRangeCount = rangeCount;

        m_computeGroups = assets.computeGroups;
        m_cullingCompute = registry->m_programRegistry->getComputeProgram(
            CS_FRUSTUM_CULLING,
            {
                { DEF_FRUSTUM_DEBUG, std::to_string(assets.frustumDebug) },
                { DEF_CS_GROUP_X, std::to_string(m_computeGroups[0])},
                { DEF_CS_GROUP_Y, std::to_string(m_computeGroups[1]) },
                { DEF_CS_GROUP_Z, std::to_string(m_computeGroups[2]) },
            });

        m_cullingCompute->prepare(assets);

        {
            constexpr int storageFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT;
            constexpr int mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;

            m_drawParameters.createEmpty(rangeCount * sizeof(gl::DrawIndirectParameters), storageFlags);
            m_drawParameters.map(mapFlags);
        }

        {
            constexpr int storageFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            constexpr int mapFlags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

            m_performanceCounters.createEmpty(rangeCount * sizeof(gl::PerformanceCounters), storageFlags);
            m_performanceCounters.map(mapFlags);

            auto* data = (gl::PerformanceCounters*)m_performanceCounters.m_data;
            *data = { 0, 0 };
        }

        m_commands = std::make_unique<GLCommandQueue>(
            "drawCommand",
            commandBatchCount,
            commandRangeCount,
            m_useFence,
            m_useSingleFence);
        m_commands->prepare(BUFFER_ALIGNMENT, assets.batchDebug);

        m_drawRanges.reserve(rangeCount);
        for (int i = 0; i < rangeCount; i++) {
            m_drawRanges.emplace_back();
        }
    }

    void DrawBuffer::bind()
    {
        if (m_bound) return;
        m_bound = true;

        m_drawParameters.bindParameter();
        m_commands->m_buffer.bindDrawIndirect();

        m_drawParameters.bindSSBO(SSBO_DRAW_PARAMETERS);
        m_performanceCounters.bindSSBO(SSBO_PERFORMANCE_COUNTERS);

        m_commands->m_buffer.bindSSBO(SSBO_DRAW_COMMANDS);
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
        const size_t drawCount = cmdRange.m_usedCount;

        if (drawCount == 0) return;

        m_commands->flush();

        // NOTE KI
        // - bind CS program
        // - execute CS program
        // - glMemoryBarrier
        // - bind draw program
        // - execute draw program

        constexpr size_t PARAMS_SZ = sizeof(gl::DrawIndirectParameters);
        const size_t paramsOffset = cmdRange.m_index * PARAMS_SZ;
        if (m_frustumGPU) {
            gl::DrawIndirectParameters params{
                (GLuint)cmdRange.m_baseIndex,
                util::as_integer(drawRange.m_drawOptions->type),
                (GLuint)drawCount
            };

            auto* data = (gl::DrawIndirectParameters*)m_drawParameters.m_data;
            data += cmdRange.m_index;

            // NOTE KI memcpy is *likely* faster than assignment operator
            memcpy(data, &params, PARAMS_SZ);

            m_drawParameters.flushRange(paramsOffset, PARAMS_SZ);
        }

        if (m_frustumGPU) {
            m_cullingCompute->bind(*drawRange.m_state);

            m_cullingCompute->u_drawParametersIndex->set(cmdRange.m_index);

            const int maxX = m_computeGroups[0];
            int groupX = drawCount;
            int groupY = 1;
            if (drawCount > maxX) {
                groupX = maxX;
                groupY = drawCount / maxX;
                if (drawCount % maxX != 0) groupY++;
            }

            glDispatchCompute(m_computeGroups[0], groupY, 1);
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
                curr.m_drawOptions->isSameMultiDraw(
                    *sendRange.m_drawOptions,
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

    void DrawBuffer::drawPending(bool drawCurrent)
    {
        if (m_frustumGPU) {
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
        }

        size_t count = 0;
        auto handler = [this, &count](GLBufferRange& cmdRange) {
            auto& drawRange = m_drawRanges[cmdRange.m_index];
            auto drawOptions = *drawRange.m_drawOptions;
            const GLsizei drawCount = (GLsizei)cmdRange.m_usedCount;

            bindDrawRange(drawRange);

            if (drawOptions.type == backend::DrawOptions::Type::elements) {
                glMultiDrawElementsIndirect(
                    drawOptions.mode,
                    GL_UNSIGNED_INT,
                    (void*)cmdRange.m_baseOffset,
                    drawCount,
                    sizeof(backend::gl::DrawIndirectCommand));
            }
            else if (drawOptions.type == backend::DrawOptions::Type::arrays)
            {
                glMultiDrawArraysIndirect(
                    drawOptions.mode,
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

    gl::PerformanceCounters DrawBuffer::getCounters(bool clear)
    {
        gl::PerformanceCounters counters;

        auto* data = (gl::PerformanceCounters*)m_performanceCounters.m_data;
        counters = *data;

        if (clear) {
            *data = { 0, 0 };
        }

        return counters;
    }

    void DrawBuffer::bindDrawRange(
        const backend::DrawRange& drawRange) const
    {
        auto& drawOptions = *drawRange.m_drawOptions;
        auto& state = *drawRange.m_state;

        drawRange.m_program->bind(state);
        state.bindVAO(*drawRange.m_vao);

        state.setEnabled(GL_CULL_FACE, !drawOptions.renderBack);

        const bool wireframe = drawOptions.wireframe || drawRange.m_forceWireframe;

        if (wireframe) {
            state.polygonFrontAndBack(GL_LINE);
        }
        else {
            state.polygonFrontAndBack(GL_FILL);
        }

        if (drawOptions.tessellation) {
            glPatchParameteri(GL_PATCH_VERTICES, drawOptions.patchVertices);
        }

        const bool blend = !wireframe && (drawOptions.blend || drawOptions.blendOIT) && drawRange.m_allowBlend;
        state.setEnabled(GL_BLEND, blend);
        if (blend) {
            if (!drawOptions.blendOIT) {
                state.setBlendMode({ GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });
            }
        }
    }
}
