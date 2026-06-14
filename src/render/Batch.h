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

        // Compute frustum visibility once for ctx.m_camera, before any pass
        // builds batches for that camera. Must be called by every batch-building
        // root (NodeDraw, shadow cascades, object-id) prior to drawing.
        // Cull this layer's drawables (ctx.m_collection / ctx.m_layer) for ctx's camera.
        // @param selector optional base filter folded into VISIBLE_SELECTED (nullptr = accept-all).
        void cullFrustum(
            const RenderContext& ctx,
            const std::function<bool(const render::DrawableInfo&)>* selector = nullptr);
        // Cull only this layer's shadow-caster groups (for the shadow cascade pass).
        void cullShadowFrustum(
            const RenderContext& ctx,
            const std::function<bool(const render::DrawableInfo&)>* selector = nullptr);

        void setInstanceRegistry(InstanceRegistry* instanceRegistry) {
            m_instanceRegistry = instanceRegistry;
        }

        void addMeshes(
            const RenderContext& ctx,
            const util::BufferReference instanceRef,
            uint8_t kindBits) noexcept;

        // emit a single drawable (per-drawable sweep path); the caller has already
        // resolved + prepared programId and applied all filtering
        void addDrawable(
            uint32_t instanceIndex,
            const render::DrawableInfo& drawable,
            ki::program_id programId) noexcept;

        // accumulate per-drawable-sweep skips (bucket candidates not emitted: culled or
        // filtered). addDrawable counts draws; this counts the rest, so draw+skip == the
        // candidates a sweep scanned. Called serially on RT from CollectionRender.
        void addSkip(size_t count) noexcept { m_skipCount += count; }

        void bind() noexcept;

        bool isFlushed() const noexcept;

        void clearBatches() noexcept;

        size_t flush(const RenderContext& ctx);

        backend::gl::PerformanceCounters getCounters(bool clear) const;
        backend::gl::PerformanceCounters getCountersLocal(bool clear) const;

    private:
        bool m_prepared{ false };

        bool m_frustumGPU{ false };
        bool m_frustumCPU{ false };
        bool m_lodDistanceEnabled{ false };

        uint32_t m_frustumParallelLimit{ 100 };

        InstanceRegistry* m_instanceRegistry{ nullptr };

        std::unique_ptr<backend::DrawBuffer> m_draw{ nullptr };

        BatchRegistry m_batchRegistry;

        MultiDrawEntryContainer m_drawEntryContainer;

        std::vector<render::InstanceIndexSSBO> m_instanceIndeces;

        size_t m_pendingCount{ 0 };
        size_t m_flushedTotalCount{ 0 };
        size_t m_frameFlushCount{ 0 };

        mutable size_t m_drawCount{ 0 };
        mutable size_t m_skipCount{ 0 };
    };
}
