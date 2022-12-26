#include "DrawBuffer.h"

#include "asset/Shader.h"


namespace backend {
    DrawBuffer::DrawBuffer()
    {
    }

    void DrawBuffer::prepare(int entryCount)
    {
        KI_GL_CHECK("1.1");

        m_entryCount = entryCount;
        m_rangeSize = entryCount * sizeof(backend::DrawIndirectCommand);

        m_buffer.create();

        m_buffer.initEmpty(
            RANGE_COUNT * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

        m_mapped = (DrawIndirectCommand*)m_buffer.map(
            0,
            RANGE_COUNT * m_rangeSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

        for (int i = 0; i < RANGE_COUNT; i++) {
            m_ranges[i].m_index = i * m_entryCount;
            m_ranges[i].m_offset = i * m_rangeSize;
        }

        KI_GL_CHECK("1.2");
    }

    void DrawBuffer::bind()
    {
        m_buffer.bindDrawIndirect();
    }

    void DrawBuffer::draw(
        GLState& state,
        const Shader* shader,
        const GLVertexArray* vao,
        const DrawOptions& drawOptions)
    {
        if (m_size == 0) return;

        glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

        shader->bind(state);
        state.useVAO(*vao);
        bindOptions(state, drawOptions);

        auto& range = m_ranges[m_range];

        wait(m_range);

        if (drawOptions.type == backend::DrawOptions::Type::elements) {
            glMultiDrawElementsIndirect(
                drawOptions.mode,
                GL_UNSIGNED_INT,
                (void*)range.m_offset,
                m_size,
                sizeof(backend::DrawIndirectCommand));
        }
        else if (drawOptions.type == backend::DrawOptions::Type::arrays)
        {
            glMultiDrawArraysIndirect(
                drawOptions.mode,
                (void*)range.m_offset,
                m_size,
                sizeof(backend::DrawIndirectCommand));
        }

        lock(m_range);

        m_range = (m_range + 1) % RANGE_COUNT;
        m_size = 0;
    }

    void DrawBuffer::send(backend::DrawIndirectCommand& indirect)
    {
        auto& range = m_ranges[m_range];
        m_mapped[range.m_index + m_size] = indirect;
        m_size++;
    }

    void DrawBuffer::lock(int index)
    {
        auto& range = m_ranges[index];
        if (range.m_sync) {
            glDeleteSync(range.m_sync);
        }
        range.m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
    void DrawBuffer::wait(int index)
    {
        auto& range = m_ranges[index];
        if (!range.m_sync) return;

        int count = 0;
        GLenum res = GL_UNSIGNALED;
        while (res != GL_ALREADY_SIGNALED && res != GL_CONDITION_SATISFIED)
        {
            res = glClientWaitSync(range.m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 100000);
            count++;
        }
        //std::cout << "waitcount: " << count << '\n';
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
