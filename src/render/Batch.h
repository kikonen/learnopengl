#pragma once

#include <span>

#include "backend/gl/PerformanceCounters.h"

#include "BatchCommand.h"

namespace backend {
    class DrawBuffer;
}

class Program;

struct Snapshot;

struct PrepareContext;
class RenderContext;

class Node;

namespace render {
    // NOTE KI use single shared UBO buffer for rendering
    // => less resources needed
    //
    // https://stackoverflow.com/questions/15438605/can-a-vbo-be-bound-to-multiple-vaos
    // https://www.khronos.org/opengl/wiki/Vertex_Specification#Index_buffers
    //
    class Batch final
    {
    public:
        Batch();
        ~Batch();

        // https://stackoverflow.com/questions/7823845/disable-compiler-generated-copy-assignment-operator
        Batch(const Batch&) = delete;
        Batch& operator=(const Batch&) = delete;

        void addSnapshot(
            const RenderContext& ctx,
            const Snapshot& snapshot,
            uint32_t entityIndex) noexcept;

        void addSnapshots(
            const RenderContext& ctx,
            const std::span<const Snapshot>& snapshots,
            const std::span<uint32_t>& entityIndeces) noexcept;

        void addSnapshotsInstanced(
            const RenderContext& ctx,
            const std::span<const Snapshot>& snapshots,
            uint32_t entityBase) noexcept;

        void bind() noexcept;

        void prepareRT(
            const PrepareContext& ctx,
            int entryCount = -1,
            int bufferCount = -1);

        void draw(
            const RenderContext& ctx,
            Node& node,
            Program* program);

        bool isFlushed() const noexcept
        {
            return m_entityIndeces.size() == 0;
        }

        void flush(
            const RenderContext& ctx);

        backend::gl::PerformanceCounters getCounters(bool clear) const;
        backend::gl::PerformanceCounters getCountersLocal(bool clear) const;

    private:
        void addCommand(
            const RenderContext& ctx,
            const kigl::GLVertexArray* vao,
            const backend::DrawOptions& drawOptions,
            Program* program) noexcept;

        bool inFrustum(
            const RenderContext& ctx,
            const Snapshot& snapshot) const noexcept;

    private:
        bool m_prepared = false;

        bool m_frustumCPU = false;
        bool m_frustumGPU = false;
        uint32_t m_frustumParallelLimit = 999;

        std::vector<BatchCommand> m_batches;

        std::vector<GLuint> m_entityIndeces;

        std::unique_ptr<backend::DrawBuffer> m_draw;

        mutable unsigned long m_drawCount = 0;
        mutable unsigned long m_skipCount = 0;
    };
}
