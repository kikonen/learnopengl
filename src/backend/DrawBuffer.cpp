#include "DrawBuffer.h"

#include "asset/Shader.h"


namespace backend {
    constexpr int BUFFER_ALIGNMENT = 1;

    DrawBuffer::DrawBuffer()
    {
    }

    void DrawBuffer::prepare(int entryCount, int rangeCount)
    {
        m_queue = std::make_unique<GLDrawSyncQueue>(
            "drawIndirect",
            entryCount,
            rangeCount,
            true);
        m_queue->prepare(BUFFER_ALIGNMENT);
    }

    void DrawBuffer::bind()
    {
        m_queue->m_buffer.bindDrawIndirect();
    }

    void DrawBuffer::flushIfNeeded(
        GLState& state,
        const Shader* shader,
        const GLVertexArray* vao,
        const DrawOptions& drawOptions,
        const bool useBlend)
    {
        if (!m_queue->isFull()) return;
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
        auto& range = m_queue->current();
        if (range.m_count == 0) return;

        m_queue->flush();

        shader->bind(state);
        state.bindVAO(*vao);
        bindOptions(state, drawOptions, useBlend);

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            glMultiDrawElementsIndirect(
                drawOptions.mode,
                GL_UNSIGNED_INT,
                (void*)range.m_baseOffset,
                range.m_count,
                sizeof(backend::DrawIndirectCommand));
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            glMultiDrawArraysIndirect(
                drawOptions.mode,
                (void*)range.m_baseOffset,
                range.m_count,
                sizeof(backend::DrawIndirectCommand));
        }

        m_queue->next(true);
        assert(m_queue->current().isEmpty());
    }

    void DrawBuffer::send(
        const backend::DrawIndirectCommand& indirect)
    {
        m_queue->send(indirect);
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