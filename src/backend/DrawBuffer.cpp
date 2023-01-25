#include "DrawBuffer.h"

#include "asset/Assets.h"
#include "asset/Shader.h"
#include "asset/SSBO.h"

#include "util/Util.h"

#include "registry/ShaderRegistry.h"

#include "backend/gl/DrawIndirectParameters.h"
#include "DrawOptions.h"


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

        m_batchCount = batchCount;
        m_rangeCount = rangeCount;

        int batchMultiplier = 1;

        //batchMultiplier = rangeCount;
        //rangeCount = 1;

        int candidateBatchCount = batchCount * batchMultiplier;
        int commandBatchCount = batchCount * batchMultiplier;

        int candidateRangeCount = rangeCount;
        int commandRangeCount = rangeCount;

        m_cullingCompute = shaders.getComputeShader(CS_FRUSTUM_CULLING);
        m_cullingCompute->prepare(assets);

        m_commands = std::make_unique<GLCommandQueue>(
            "drawCandidate",
            candidateBatchCount,
            candidateRangeCount);
        m_commands->prepare(BUFFER_ALIGNMENT);

        {
            int storageFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT;
            int mapFlags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;

            m_drawParameters.createEmpty(rangeCount * sizeof(gl::DrawIndirectParameters), storageFlags);
            m_drawParameters.map(mapFlags);
        }

        m_commands = std::make_unique<GLCommandQueue>(
            "drawCommand",
            commandBatchCount,
            commandRangeCount);
        m_commands->prepare(BUFFER_ALIGNMENT);
    }

    void DrawBuffer::bind()
    {
        if (m_bound) return;
        m_bound = true;

        m_drawParameters.bindParameter();
        m_commands->m_buffer.bindDrawIndirect();

        m_drawParameters.bindSSBO(SSBO_DRAW_PARAMETERS);

        m_commands->m_buffer.bindSSBO(SSBO_DRAW_COMMANDS);
    }

    void DrawBuffer::flushIfNeeded(
        const backend::DrawRange& drawRange)
    {
        if (!m_commands->full()) return;
        //std::cout << "-F-";
        flush(drawRange);
    }

    void DrawBuffer::flush(
        const backend::DrawRange& drawRange)
    {
        const auto& cmdRange = m_commands->current();
        const size_t drawCount = cmdRange.m_usedCount;

        if (drawCount == 0) return;

        m_commands->flush();

        // TODO KI
        // - bind CS shader
        // - execute CS shader
        // - glMemoryBarrier
        // - bind draw shader
        // - execute draw shader

        constexpr size_t PARAMS_SZ = sizeof(gl::DrawIndirectParameters);
        const size_t paramsOffset = cmdRange.m_index * PARAMS_SZ;
        {
            gl::DrawIndirectParameters params{
                cmdRange.m_baseIndex,
                util::as_integer(drawRange.drawOptions->type)
            };

            auto* data = (gl::DrawIndirectParameters*)m_drawParameters.m_data;
            data += cmdRange.m_index;
            // NOTE KI memcpy is *likely* faster than assignment operator
            memcpy(data, &params, PARAMS_SZ);

            m_drawParameters.flushRange(paramsOffset, PARAMS_SZ);
        }
        {
            m_cullingCompute->bind(*drawRange.state);
            m_cullingCompute->u_drawParametersIndex.set(cmdRange.m_index);

            glDispatchCompute(drawCount, 1, 1);
        }

        // NOTE KI for "non count" version sync for getting count has done this
        //glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

        bindDrawRange(drawRange);

        if (drawRange.drawOptions->type == backend::DrawOptions::Type::elements) {
            glMultiDrawElementsIndirect(
                drawRange.drawOptions->mode,
                GL_UNSIGNED_INT,
                (void*)cmdRange.m_baseOffset,
                drawCount,
                sizeof(backend::gl::DrawIndirectCommand));
        }
        else if (drawRange.drawOptions->type == backend::DrawOptions::Type::arrays)
        {
            glMultiDrawArraysIndirect(
                drawRange.drawOptions->mode,
                (void*)cmdRange.m_baseOffset,
                drawCount,
                sizeof(backend::gl::DrawIndirectCommand));
        }

        m_commands->next(true);
    }

    void DrawBuffer::send(
        const backend::DrawRange& drawRange,
        const backend::gl::DrawIndirectCommand& cmd)
    {
        m_commands->send(cmd);
        flushIfNeeded(drawRange);
    }

    void DrawBuffer::bindDrawRange(
        const backend::DrawRange& drawRange) const
    {
        auto& drawOptions = *drawRange.drawOptions;
        auto& state = *drawRange.state;

        drawRange.shader->bind(state);
        state.bindVAO(*drawRange.vao);

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

        if (drawOptions.blend && drawRange.useBlend) {
            state.setBlendMode({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE });
            state.enable(GL_BLEND);
        }
        else {
            state.disable(GL_BLEND);
        }
    }
}
