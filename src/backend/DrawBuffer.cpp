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
            m_rangeCount * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
        m_mapped = (DrawIndirectCommand*)m_buffer.mapRange(
            0,
            m_rangeCount * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);

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

        shader->bind(state);
        state.useVAO(*vao);
        bindOptions(state, drawOptions);

        auto& range = m_ranges[m_index];

        m_buffer.flushRange(range.m_offset, m_count * sizeof(backend::DrawIndirectCommand));

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            glMultiDrawElementsIndirect(
                drawOptions.mode,
                GL_UNSIGNED_INT,
                (void*)range.m_offset,
                m_count,
                sizeof(backend::DrawIndirectCommand));
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            glMultiDrawArraysIndirect(
                drawOptions.mode,
                (void*)range.m_offset,
                m_count,
                sizeof(backend::DrawIndirectCommand));
        }

        lock(m_index);

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
        wait(m_index);

        if (range.m_sync) {
            glDeleteSync(range.m_sync);
            range.m_sync = 0;
        }

        m_mapped[range.m_index + m_count] = indirect;
        m_count++;

        if (m_count == m_rangeCount) {
            flush(state, shader, vao, drawOptions);
        }
    }

    void DrawBuffer::lock(int index)
    {
        if (index != m_rangeCount - 1) return;
        auto& range = m_ranges[0];

        if (range.m_sync) {
            glDeleteSync(range.m_sync);
        }
        range.m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
    void DrawBuffer::wait(int index)
    {
        //std::cout << '.';
        if (index != 0) return;
        auto& range = m_ranges[0];

        if (!range.m_sync) return;

        int count = 0;
        GLenum res = GL_UNSIGNALED;
        while (res != GL_ALREADY_SIGNALED && res != GL_CONDITION_SATISFIED)
        {
            res = glClientWaitSync(range.m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 10000000);
            count++;
        }
        //std::cout << '-' << count;
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
