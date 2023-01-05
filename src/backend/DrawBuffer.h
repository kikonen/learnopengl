#pragma once

#include "ki/GL.h"

#include "kigl/GLState.h"
#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"
#include "kigl/GLBufferRange.h"

#include "DrawElementsIndirectCommand.h"
#include "DrawIndirectCommand.h"
#include "DrawOptions.h"

class Shader;

namespace backend {
    class DrawBuffer {
    public:
        DrawBuffer();

        void prepare(int entryCount, int rangeCount);
        void bind();

        void send(
            backend::DrawIndirectCommand& cmd,
            GLState& state,
            const Shader* shader,
            const GLVertexArray* vao,
            const DrawOptions& drawOptions,
            const bool useBlend);

        void flush(
            GLState& state,
            const Shader* shader,
            const GLVertexArray* vao,
            const DrawOptions& drawOptions,
            const bool useBlend);

    private:
        void bindOptions(
            GLState& state,
            const DrawOptions& drawOptions,
            const bool useBlend) const;

    private:
        int m_entryCount = 0;
        int m_rangeCount = 0;
        int m_rangeSize = 0;

        GLBuffer m_buffer;
        backend::DrawIndirectCommand* m_mapped;

        int m_index = 0;
        std::vector<GLBufferRange> m_ranges;
    };
}
