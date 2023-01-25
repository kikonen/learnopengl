#pragma once

#include "ki/GL.h"

#include "kigl/GLState.h"
#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "kigl/GLSyncQueue.h"

#include "gl/DrawIndirectCommand.h"

#include "DrawRange.h"
#include "DrawOptions.h"

class Assets;
class Shader;
class ShaderRegistry;

namespace backend {
    using GLCommandQueue = GLSyncQueue<backend::gl::DrawIndirectCommand, true, true>;

    class DrawBuffer {
    public:
        DrawBuffer();

        void prepare(
            const Assets& assets,
            ShaderRegistry& shaders,
            int batchCount,
            int rangeCount);

        void bind();

        void send(
            const backend::DrawRange& drawRange,
            const backend::gl::DrawIndirectCommand& cmd);

        void flushIfNeeded(
            const backend::DrawRange& drawRange);

        void flush(
            const backend::DrawRange& drawRange);

    private:
        void bindDrawRange(
            const backend::DrawRange& drawRange) const;

    public:
        size_t m_drawCount = 0;
        size_t m_skipCount = 0;

    private:
        int m_batchCount = 0;
        int m_rangeCount = 0;

        bool m_bound = false;

        Shader* m_cullingCompute{ nullptr };

        std::unique_ptr<GLCommandQueue> m_commands{ nullptr };

        GLBuffer m_drawParameters{ "drawCommandCounter" };
    };
}
