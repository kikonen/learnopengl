#include "DrawBuffer.h"

#include "asset/Assets.h"
#include "asset/Shader.h"
#include "asset/SSBO.h"

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
        m_batchCount = batchCount;
        //rangeCount = 1;

        int candidateBatchCount = batchCount * rangeCount;
        int commandBatchCount = batchCount * rangeCount;

        int candidateRangeCount = 1;
        int commandRangeCount = 1;

        m_candidateShader = shaders.getComputeShader(CS_FRUSTUM_CULLING);
        m_candidateShader->prepare(assets);

        m_candidates = std::make_unique<GLCandidateQueue>(
            "drawCandidate",
            candidateBatchCount,
            candidateRangeCount);
        m_candidates->prepare(BUFFER_ALIGNMENT);

        m_commandCounter.createEmpty(sizeof(GLuint), GL_DYNAMIC_STORAGE_BIT);

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

        //m_commandCounter.bindAtomicCounter(SSBO_DRAW_COMMAND_COUNTER);
        m_commandCounter.bindSSBO(SSBO_DRAW_COMMAND_COUNTER);
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
        auto& range = m_candidates->current();
        if (range.m_count == 0) return;

        m_candidates->flush();

        // TODO KI
        // - bind CS shader
        // - execute CS shader
        // - glMemoryBarrier
        // - bind draw shader
        // - execute draw shader

        GLuint count = 0;
        {
            m_candidateShader->bind(state);

            m_commandCounter.update(0, sizeof(GLuint), &count);

            m_candidates->bindSSBO(SSBO_CANDIDATE_DRAWS);
            m_commands->bindSSBO(SSBO_DRAW_COMMANDS);

            glDispatchCompute(range.m_count, 1, 1);

            glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
        }

        m_commandCounter.get(&count);
        if (false) {
            std::cout << "[draw=" << count<< "]";
            //KI_DEBUG(fmt::format(
            //    "DRAW: type={}, range={}, count={}",
            //    util::as_integer(drawOptions.type), range.m_count, count));
        }

        shader->bind(state);
        state.bindVAO(*vao);
        bindOptions(state, drawOptions, useBlend);

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            //std::cout << "[e" << range.m_count << "]";
            if (false) {
                glMultiDrawElementsIndirect(
                    drawOptions.mode,
                    GL_UNSIGNED_INT,
                    (void*)range.m_baseOffset,
                    count, //range.m_count,
                    sizeof(backend::DrawIndirectCommand));
            }
            else {
                glMultiDrawElementsIndirectCount(
                    drawOptions.mode,
                    GL_UNSIGNED_INT,
                    (void*)range.m_baseOffset,
                    0,
                    range.m_count,
                    sizeof(backend::DrawIndirectCommand));
            }
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            //std::cout << "[a" << range.m_count << "]";
            if (false) {
                glMultiDrawArraysIndirect(
                    drawOptions.mode,
                    (void*)range.m_baseOffset,
                    count, //range.m_count,
                    sizeof(backend::DrawIndirectCommand));
            } else {
                glMultiDrawArraysIndirectCount(
                    drawOptions.mode,
                    (void*)range.m_baseOffset,
                    0,
                    range.m_count,
                    sizeof(backend::DrawIndirectCommand));
            }
        }

        m_candidates->next(true);
        m_commands->next(true);

        assert(m_candidates->current().isEmpty());
        assert(m_commands->current().isEmpty());
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
