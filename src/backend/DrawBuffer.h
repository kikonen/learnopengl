#pragma once

#include "ki/GL.h"

#include "kigl/GLState.h"
#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

#include "kigl/GLSyncQueue.h"

#include "gl/CandidateDraw.h"
#include "gl/DrawIndirectCommand.h"

#include "DrawOptions.h"

class Assets;
class Shader;
class ShaderRegistry;

namespace backend {
    using GLCandidateQueue = GLSyncQueue<backend::gl::CandidateDraw, true, true>;
    // NOTE KI updated by compute shader, just need to sync with candidate draw swaps
    using GLCommandQueue = GLSyncQueue<backend::gl::DrawIndirectCommand, false, false>;

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
            const backend::gl::CandidateDraw& cmd);

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

    public:
        size_t m_drawCount = 0;
        size_t m_skipCount = 0;

    private:
        int m_batchCount = 0;
        int m_rangeCount = 0;

        bool m_bound = false;

        bool m_useIndirectCount;

        Shader* m_candidateShader{ nullptr };

        std::unique_ptr<GLCandidateQueue> m_candidates{ nullptr };

        std::unique_ptr<GLCommandQueue> m_commands{ nullptr };

        GLBuffer m_commandCounter{ "drawCommandCounter" };
    };
}
