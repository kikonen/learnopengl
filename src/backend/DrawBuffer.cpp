#include "DrawBuffer.h"

#include "asset/Shader.h"


namespace backend {
    DrawBuffer::DrawBuffer()
    {
    }

    void DrawBuffer::prepare(int entryCount, int rangeCount)
    {
        KI_GL_CHECK("1.1");

        m_entryCount = entryCount;
        m_rangeCount = rangeCount;
        m_rangeSize = entryCount * sizeof(backend::DrawIndirectCommand);

        m_buffer.create();

        m_buffer.initEmpty(
            m_rangeCount * m_rangeSize, GL_DYNAMIC_STORAGE_BIT);

        m_entries.reserve(m_rangeCount * m_entryCount);

        for (int i = 0; i < m_rangeCount * m_entryCount; i++) {
            m_entries.emplace_back();
        }

        for (int i = 0; i < m_rangeCount ; i++) {
            auto& range = m_ranges.emplace_back();
            range.m_index = i * m_entryCount;
            range.m_offset = i * m_rangeSize;
        }

        KI_GL_CHECK("1.2");
    }

    void DrawBuffer::bind()
    {
        m_buffer.bindDrawIndirect();
    }

    void DrawBuffer::flush(
        GLState& state,
        const Shader* shader,
        const GLVertexArray* vao,
        const DrawOptions& drawOptions)
    {
        if (m_count == 0) return;

        // NOTE KI try to wait before changing shader
        //glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
        KI_GL_CHECK("1.1");

        shader->bind(state);
        state.useVAO(*vao);
        bindOptions(state, drawOptions);

        auto& range = m_ranges[m_index];

        KI_GL_CHECK("1.1");
        m_buffer.update(range.m_offset, m_count * sizeof(backend::DrawIndirectCommand), &m_entries[range.m_index]);
        KI_GL_CHECK("1.2");

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            glMultiDrawElementsIndirect(
                drawOptions.mode,
                GL_UNSIGNED_INT,
                (void*)range.m_offset,
                m_count,
                sizeof(backend::DrawIndirectCommand));
            KI_GL_CHECK("1.2.1");
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            glMultiDrawArraysIndirect(
                drawOptions.mode,
                (void*)range.m_offset,
                m_count,
                sizeof(backend::DrawIndirectCommand));
            KI_GL_CHECK("1.2.2");
        }

        KI_GL_CHECK("1.3");

        m_index = (m_index + 1) % m_ranges.size();
        m_count = 0;
    }

    void DrawBuffer::send(
        backend::DrawIndirectCommand& indirect,
        GLState& state,
        const Shader* shader,
        const GLVertexArray* vao,
        const DrawOptions& drawOptions)
    {
        auto& range = m_ranges[m_index];

        m_entries[range.m_index + m_count] = indirect;
        m_count++;

        if (m_count == m_rangeCount) {
            flush(state, shader, vao, drawOptions);
        }
    }

    void DrawBuffer::bindOptions(
        GLState& state,
        const DrawOptions& drawOptions) const
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

        if (drawOptions.blend) {
            // NOTE KI FrameBufferAttachment::getTextureRGB() also fixes this
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
             glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

            state.enable(GL_BLEND);
        }
        else {
            state.disable(GL_BLEND);
        }
    }
}
