#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <span>

#include "ki/ki.h"

#include "backend/gl/PerformanceCounters.h"

#include "render/BatchRegistry.h"
#include "render/InstanceIndexSSBO.h"

namespace backend {
    class DrawBuffer;
}

namespace mesh {
    struct MeshInstance;
}

namespace model {
    class Node;
    class NodeType;
}

struct PrepareContext;
struct UpdateContext;

namespace util {
    struct BufferReference;
}

namespace render {
    class RenderContext;
    class InstanceRegistry;
    struct DrawableInfo;
    struct MultiDrawEntry;

    class Batch {
    public:
        Batch();
        ~Batch();

        void prepareRT(
            const PrepareContext& ctx,
            int entryCount = 0,
            int bufferCount = 0);

        void updateRT(const UpdateContext& ctx);

        void beginFrame();
        void endFrame();

        void setInstanceRegistry(InstanceRegistry* instanceRegistry) {
            m_instanceRegistry = instanceRegistry;
        }

        void addDrawablesSingleNode(
            const RenderContext& ctx,
            const model::NodeType* type,
            const util::BufferReference instanceRef,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            uint8_t kindBits) noexcept;

        void addDrawablesInstanced(
            const RenderContext& ctx,
            const model::NodeType* type,
            const util::BufferReference instanceRef,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            uint8_t kindBits) noexcept;

        void addMesh(
            const RenderContext& ctx,
            uint8_t kindBits,
            const mesh::MeshInstance& instance,
            ki::program_id defaultProgramId,
            uint32_t entityIndex) noexcept;

        void bind() noexcept;

        void draw(
            const RenderContext& ctx,
            model::Node* node,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            uint8_t kindBits);

        bool isFlushed() const noexcept;

        void clearBatches() noexcept;

        size_t flush(const RenderContext& ctx);

        backend::gl::PerformanceCounters getCounters(bool clear) const;
        backend::gl::PerformanceCounters getCountersLocal(bool clear) const;

    private:
        bool m_prepared{ false };

        bool m_frustumCPU{ false };
        bool m_frustumGPU{ false };
        uint32_t m_frustumParallelLimit{ 100 };

        InstanceRegistry* m_instanceRegistry{ nullptr };

        std::unique_ptr<backend::DrawBuffer> m_draw{ nullptr };

        BatchRegistry m_batchRegistry;

        std::vector<MultiDrawEntry> m_pending;

        std::vector<render::InstanceIndexSSBO> m_instanceIndeces;

        size_t m_pendingCount{ 0 };
        size_t m_flushedTotalCount{ 0 };

        mutable size_t m_drawCount{ 0 };
        mutable size_t m_skipCount{ 0 };
    };
}
