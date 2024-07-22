#pragma once

#include <span>
#include <functional>

#include "backend/gl/PerformanceCounters.h"

#include "BatchCommand.h"

#include "mesh/InstanceSSBO.h"

namespace backend {
    class DrawBuffer;
}

namespace mesh {
    class MeshType;
    struct LodMesh;
}

class Program;

struct Snapshot;

struct PrepareContext;
struct UpdateContext;
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
            const std::function<Program* (const mesh::LodMesh&)>& programSelector,
            uint8_t kindBits,
            const Snapshot& snapshot,
            uint32_t entityIndex) noexcept;

        //void addSnapshots(
        //    const RenderContext& ctx,
        //    mesh::MeshType* type,
        //    std::span<const Snapshot> snapshots,
        //    std::span<uint32_t> entityIndeces) noexcept;

        // NOTE KI snapshots starting from entityBaseIndex
        void addSnapshotsInstanced(
            const RenderContext& ctx,
            const mesh::MeshType* type,
            const std::function<Program* (const mesh::LodMesh&)>& programSelector,
            uint8_t kindBits,
            std::span<const Snapshot> snapshots,
            uint32_t entityBaseIndex) noexcept;

        void bind() noexcept;

        void prepareRT(
            const PrepareContext& ctx,
            int entryCount = -1,
            int bufferCount = -1);

        void updateRT(
            const UpdateContext& ctx);

        void draw(
            const RenderContext& ctx,
            mesh::MeshType* type,
            const std::function<Program* (const mesh::LodMesh&)>& programSelector,
            uint8_t kindBits,
            Node& node);

        bool isFlushed() const noexcept;

        void clearBatches() noexcept;

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

        backend::DrawBuffer* getDrawBuffer()
        {
            return m_draw.get();
        }

    private:
        bool m_prepared = false;

        bool m_frustumCPU = false;
        bool m_frustumGPU = false;
        uint32_t m_frustumParallelLimit = 999;

        size_t m_pendingCount{ 0 };
        std::map<BatchKey, BatchCommand> m_batches;

        std::vector<mesh::InstanceSSBO> m_entityIndeces;

        std::unique_ptr<backend::DrawBuffer> m_draw;

        mutable size_t m_drawCount{ 0 };
        mutable size_t m_skipCount{ 0 };

        size_t m_flushedTotalCount{ 0 };
    };
}
