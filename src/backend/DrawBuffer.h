#pragma once

#include "ki/GL.h"

#include "kigl/GLState.h"
#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "kigl/GLSyncQueue.h"

#include "DrawElementsIndirectCommand.h"
#include "DrawIndirectCommand.h"
#include "DrawOptions.h"

class Shader;


namespace backend {
    using GLDrawSyncQueue = GLSyncQueue<backend::DrawIndirectCommand, true>;

    class DrawBuffer {
    public:
        DrawBuffer();

        void prepare(int entryCount, int rangeCount);
        void bind();

        void send(
            const backend::DrawIndirectCommand& cmd);

        void flushIfNeeded(
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
        std::unique_ptr<GLDrawSyncQueue> m_queue{ nullptr };
    };
}
