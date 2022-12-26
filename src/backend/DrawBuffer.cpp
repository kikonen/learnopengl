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

        constexpr int flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

        m_buffer.create();
        m_buffer.initEmpty(RANGE_COUNT * m_rangeSize, flags);
        m_mapped = (DrawIndirectCommand*)m_buffer.map(0, RANGE_COUNT * m_rangeSize, flags);

        for (int i = 0; i < RANGE_COUNT; i++) {
            m_ranges[i].m_offset = i * m_rangeSize;
            m_ranges[i].m_mappedBase = i * m_entryCount;
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

        wait(m_index);

        updateState(state, drawOptions.renderBack, drawOptions.wireframe);
        shader->bind(state);
        state.useVAO(*vao);

        auto& range = m_ranges[m_index];

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

        lock(m_index);

        m_index = (m_index + 1) % RANGE_COUNT;
        m_size = 0;
    }

    void DrawBuffer::send(backend::DrawIndirectCommand& indirect)
    {
        auto& range = m_ranges[m_index];
        m_mapped[range.m_mappedBase + m_size] = indirect;
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
            res = glClientWaitSync(range.m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            count++;
        }
        //std::cout << "waitcount: " << count << '\n';
    }

    void DrawBuffer::updateState(
        GLState& state,
        bool renderBack,
        bool wireframe) const
    {
        if (renderBack) {
            state.disable(GL_CULL_FACE);
        }
        else {
            state.enable(GL_CULL_FACE);
        }

        if (wireframe) {
            state.polygonFrontAndBack(GL_LINE);
        }
        else {
            state.polygonFrontAndBack(GL_FILL);
        }
    }
}
