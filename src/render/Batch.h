#pragma once

#include <span>
#include <functional>

#include "backend/gl/PerformanceCounters.h"

#include "util/BufferReference.h"

#include "BatchCommand.h"
#include "BatchRegistry.h"

#include "render/InstanceIndexSSBO.h"

namespace backend
{
    class DrawBuffer;
}

namespace render
{
    struct DrawableInfo;
    struct InstanceSSBO;
}

namespace mesh
{
    struct LodMesh;
    struct Transform;
    struct MeshInstance;
}

namespace model
{
    class Node;
    class NodeType;
    struct Snapshot;
}

class Program;

struct PrepareContext;
struct UpdateContext;

namespace render
{
    class InstanceRegistry;
    class RenderContext;
    struct DrawableInfo;

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

        // Optimized drawing for "single node" case
        // => makes assumptions about frustum etc.
        void addDrawablesSingleNode(
            const RenderContext& ctx,
            const model::NodeType* type,
            const util::BufferReference instanceRef,
            const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            uint8_t kindBits) noexcept;

        // Drwables for "instanced" drawing
        // => every drawable is separate
        void addDrawablesInstanced(
            const RenderContext& ctx,
            const model::NodeType* type,
            const util::BufferReference instanceRef,
            const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            uint8_t kindBits) noexcept;

        void addMesh(
            const RenderContext& ctx,
            uint8_t kindBits,
            const mesh::MeshInstance& instance,
            ki::program_id programId,
            uint32_t entityIndex) noexcept;

        void bind() noexcept;

        void setInstanceRegistry(InstanceRegistry* registry)
        {
            m_instanceRegistry = registry;
        }

        void prepareRT(
            const PrepareContext& ctx,
            int entryCount = -1,
            int bufferCount = -1);

        void updateRT(
            const UpdateContext& ctx);

        void draw(
            const RenderContext& ctx,
            model::Node* node,
            const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            uint8_t kindBits);

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

        BatchRegistry m_batchRegistry;

        InstanceRegistry* m_instanceRegistry{ nullptr };

        std::vector<MultiDrawEntry> m_pending;
        size_t m_pendingCount{ 0 };

        std::vector<render::InstanceIndexSSBO> m_instanceIndeces;

        std::unique_ptr<backend::DrawBuffer> m_draw;

        mutable size_t m_drawCount{ 0 };
        mutable size_t m_skipCount{ 0 };

        size_t m_flushedTotalCount{ 0 };
    };
}
