#include "DrawBuffer.h"

#include "asset/Assets.h"
#include "asset/Shader.h"
#include "asset/SSBO.h"

#include "backend/DrawIndirectParameters.h"

#include "util/Util.h"

#include "registry/ShaderRegistry.h"

namespace backend {
    constexpr int BUFFER_ALIGNMENT = 1;

    DrawBuffer::DrawBuffer()
    {
    }

    void DrawBuffer::prepare(
        const Assets& assets,
        ShaderRegistry& shaders,
        int batchCount,
        int rangeCount)
    {
        const auto info = ki::GL::getInfo();
        m_useIndirectCount = info.vendor != "Intel";
        m_useIndirectCount = false;

        KI_INFO_OUT(fmt::format("USE_DRAW_INDIRECT_COUNT={}", m_useIndirectCount));

        m_batchCount = batchCount;
        m_rangeCount = rangeCount;

        int batchMultiplier = 1;

        //batchMultiplier = rangeCount;
        //rangeCount = 1;

        int candidateBatchCount = batchCount;
        int commandBatchCount = batchCount;

        int candidateRangeCount = rangeCount;
        int commandRangeCount = rangeCount;

        candidateBatchCount = batchCount* batchMultiplier;
        commandBatchCount = batchCount * batchMultiplier;

        candidateRangeCount = rangeCount;
        commandRangeCount = rangeCount;

        m_candidateShader = shaders.getComputeShader(CS_FRUSTUM_CULLING);
        m_candidateShader->prepare(assets);

        m_candidates = std::make_unique<GLCandidateQueue>(
            "drawCandidate",
            candidateBatchCount,
            candidateRangeCount);
        m_candidates->prepare(BUFFER_ALIGNMENT);

        m_commandCounter.createEmpty(rangeCount * sizeof(DrawIndirectParameters), GL_DYNAMIC_STORAGE_BIT);

        m_commands = std::make_unique<GLCommandQueue>(
            "drawCommand",
            commandBatchCount,
            commandRangeCount);
        m_commands->prepare(BUFFER_ALIGNMENT);
    }

    void DrawBuffer::bind()
    {
        m_commandCounter.bindParameter();
        m_commands->m_buffer.bindDrawIndirect();

        m_commandCounter.bindSSBO(SSBO_DRAW_COMMAND_COUNTER);

        m_candidates->m_buffer.bindSSBO(SSBO_CANDIDATE_DRAWS);
        m_commands->m_buffer.bindSSBO(SSBO_DRAW_COMMANDS);
    }

    void DrawBuffer::flushIfNeeded(
        GLState& state,
        const Shader* shader,
        const GLVertexArray* vao,
        const DrawOptions& drawOptions,
        const bool useBlend)
    {
        if (!m_candidates->isFull()) return;
        //std::cout << "-F-";
        flush(
            state,
            shader,
            vao,
            drawOptions,
            useBlend);
    }

    void DrawBuffer::flush(
        GLState& state,
        const Shader* shader,
        const GLVertexArray* vao,
        const DrawOptions& drawOptions,
        const bool useBlend)
    {
        const auto& candidateRange = m_candidates->current();
        const int drawCount = candidateRange.m_count;

        if (drawCount == 0) return;

        const auto& cmdRange = m_commands->current();

        m_candidates->flush();

        // TODO KI
        // - bind CS shader
        // - execute CS shader
        // - glMemoryBarrier
        // - bind draw shader
        // - execute draw shader

        const int paramsSz = sizeof(DrawIndirectParameters);
        const int paramsOffset = candidateRange.m_index * paramsSz;
        DrawIndirectParameters param{ 0, cmdRange.m_baseIndex };
        {
            m_commandCounter.update(paramsOffset, paramsSz, &param);

            m_candidateShader->bind(state);

            m_candidateShader->u_drawParametersIndex.set(candidateRange.m_index);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            glDispatchCompute(drawCount, 1, 1);
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }

        param.u_counter = -1;
        m_commandCounter.getRange(paramsOffset, paramsSz, &param);

        int count = param.u_counter;
        if (count == -1) count = 0;
        size_t skip = drawCount - count;
        m_drawCount += count;
        m_skipCount += skip;

        if (true) {
            if (count > 0) std::cout << " [draw: " << count << "]";
            if (skip > 0) std::cout << " *skip: " << skip << "*";

            //KI_DEBUG(fmt::format(
            //    "DRAW: type={}, range={}, count={}",
            //    util::as_integer(drawOptions.type), range.m_count, count));
        }

        if (count > 0) {
            shader->bind(state);
            state.bindVAO(*vao);
            bindOptions(state, drawOptions, useBlend);

            if (drawOptions.type == backend::DrawOptions::Type::elements) {
                //std::cout << "[e" << range.m_count << "]";
                if (!m_useIndirectCount) {
                    glMultiDrawElementsIndirect(
                        drawOptions.mode,
                        GL_UNSIGNED_INT,
                        (void*)cmdRange.m_baseOffset,
                        count, //range.m_count,
                        sizeof(backend::DrawIndirectCommand));
                }
                else {
                    glMultiDrawElementsIndirectCount(
                        drawOptions.mode,
                        GL_UNSIGNED_INT,
                        (void*)cmdRange.m_baseOffset,
                        paramsOffset,
                        drawCount,
                        sizeof(backend::DrawIndirectCommand));
                }
            }
            else if (drawOptions.type == backend::DrawOptions::Type::arrays)
            {
                //std::cout << "[a" << range.m_count << "]";
                if (!m_useIndirectCount) {
                    glMultiDrawArraysIndirect(
                        drawOptions.mode,
                        (void*)cmdRange.m_baseOffset,
                        count, //range.m_count,
                        sizeof(backend::DrawIndirectCommand));
                }
                else {
                    glMultiDrawArraysIndirectCount(
                        drawOptions.mode,
                        (void*)cmdRange.m_baseOffset,
                        paramsOffset,
                        drawCount,
                        sizeof(backend::DrawIndirectCommand));
                }
            }
        }

        m_candidates->next(true);
        m_commands->next(true);
    }

    void DrawBuffer::send(
        const backend::CandidateDraw& cmd)
    {
        m_candidates->send(cmd);
    }

    void DrawBuffer::bindOptions(
        GLState& state,
        const DrawOptions& drawOptions,
        const bool useBlend) const
    {
        if (drawOptions.renderBack) {
            state.disable(GL_CULL_FACE);
        }
        else {
            state.enable(GL_CULL_FACE);
        }

        if (drawOptions.wireframe) {
            state.polygonFrontAndBack(GL_LINE);
        }
        else {
            state.polygonFrontAndBack(GL_FILL);
        }

        if (drawOptions.blend && useBlend) {
            state.setBlendMode({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });
            state.enable(GL_BLEND);
        }
        else {
            state.disable(GL_BLEND);
        }
    }
}
