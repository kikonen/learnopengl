#pragma once

#include <span>

#include "backend/gl/PerformanceCounters.h"

#include "BatchCommand.h"

#include "mesh/InstanceSSBO.h"

namespace backend {
    class DrawBuffer;
}

namespace mesh {
    class MeshType;
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
            const mesh::MeshType* type,
            const backend::Lod* lod,
            const Snapshot& snapshot,
            uint32_t entityIndex) noexcept;

        //void addSnapshots(
        //    const RenderContext& ctx,
        //    mesh::MeshType* type,
        //    std::span<const Snapshot> snapshots,
        //    std::span<uint32_t> entityIndeces) noexcept;

        void addSnapshotsInstanced(
            const RenderContext& ctx,
            const mesh::MeshType* type,
            std::span<const backend::Lod*> lods,
            std::span<const Snapshot> snapshots,
            uint32_t entityBase) noexcept;

        void bind() noexcept;

        void prepareRT(
            const PrepareContext& ctx,
            int entryCount = -1,
            int bufferCount = -1);

        void draw(
            const RenderContext& ctx,
            mesh::MeshType* type,
            Node& node,
            Program* program);

        bool isFlushed() const noexcept;

        size_t getFlushedTotalCount() const noexcept {
            return m_flushedTotalCount;
        }

        void clearFlushedTotalCount() noexcept {
            m_flushedTotalCount = 0;
        }

        size_t flush(
            const RenderContext& ctx);

        backend::gl::PerformanceCounters getCounters(bool clear) const;
        backend::gl::PerformanceCounters getCountersLocal(bool clear) const;

    private:
        void addCommand(
            const RenderContext& ctx,
            const kigl::GLVertexArray* vao,
            const backend::DrawOptions& drawOptions,
            Program* program) noexcept;

    private:
        bool m_prepared = false;

        bool m_frustumCPU = false;
        bool m_frustumGPU = false;
        uint32_t m_frustumParallelLimit = 999;

        std::vector<BatchCommand> m_batches;

        std::vector<mesh::InstanceSSBO> m_entityIndeces;

        std::unique_ptr<backend::DrawBuffer> m_draw;

        mutable size_t m_drawCount{ 0 };
        mutable size_t m_skipCount{ 0 };

        size_t m_flushedTotalCount{ 0 };
    };
}
