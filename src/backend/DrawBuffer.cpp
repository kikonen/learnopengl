#include "DrawBuffer.h"

#include "asset/Shader.h"


namespace backend {
    DrawBuffer::DrawBuffer()
    {
    }

    void DrawBuffer::prepare(int entryCount, int rangeCount)
    {
        m_entryCount = entryCount;
        m_rangeCount = rangeCount;
        m_rangeSize = entryCount * sizeof(backend::DrawIndirectCommand);

        m_buffer.create();

        m_buffer.initEmpty(
            m_rangeCount * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
        m_mapped = (DrawIndirectCommand*)m_buffer.mapRange(
            0,
            m_rangeCount * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

        for (int i = 0; i < m_rangeCount ; i++) {
            auto& range = m_ranges.emplace_back();
            range.m_baseIndex = i * m_entryCount;
            range.m_maxCount = m_entryCount;
            range.m_count = 0;
            range.m_baseOffset = i * m_rangeSize;
        }
    }

    void DrawBuffer::bind()
    {
        m_buffer.bindDrawIndirect();
    }

    void DrawBuffer::flush(
        GLState& state,
        const Shader* shader,
        const GLVertexArray* vao,
        const DrawOptions& drawOptions,
        const bool useBlend)
    {
        auto& range = m_ranges[m_index];
        if (range.m_count == 0) return;

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

        range.m_count = 0;
        m_index = (m_index + 1) % m_ranges.size();

        if (m_index == 0) m_ranges[0].lock();
    }

    void DrawBuffer::send(
        backend::DrawIndirectCommand& indirect,
        GLState& state,
        const Shader* shader,
        const GLVertexArray* vao,
        const DrawOptions& drawOptions,
        const bool useBlend)
    {
        auto& range = m_ranges[m_index];
        if (m_index == 0) range.wait();

        m_mapped[range.next()] = indirect;

        if (range.isFull()) {
            flush(state, shader, vao, drawOptions, useBlend);
        }
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
