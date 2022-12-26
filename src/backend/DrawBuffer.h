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
    constexpr int RANGE_COUNT = 3;

    class DrawBuffer {
    public:
        DrawBuffer();

        void prepare(int entryCount);
        void bind();

        void send(backend::DrawIndirectCommand& cmd);

        void draw(
            GLState& state,
            const Shader* shader,
            const GLVertexArray* vao,
            const DrawOptions& drawOptions);

    private:
        void lock(int index);
        void wait(int index);

        void bindOptions(
            GLState& state,
            const DrawOptions& drawOptions) const;

    private:
        int m_entryCount = 0;
        int m_rangeSize = 0;

        GLBuffer m_buffer;
        backend::DrawIndirectCommand* m_mapped;
        int m_size = 0;

        int m_range = 0;
        GLBufferRange m_ranges[RANGE_COUNT];
    };
}
