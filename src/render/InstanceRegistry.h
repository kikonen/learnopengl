#pragma once

#include <vector>
#include <span>
#include <array>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

#include "util/BufferReference.h"
#include "util/DirtySet.h"
#include "util/SlotAllocator.h"

#include "backend/DrawOptions.h"

#include "DrawableInfo.h"

struct Frustum;

namespace render
{
    struct InstanceSSBO;

    // Per-drawable visibility flags computed once per camera by cullFrustum().
    // A bit is SET when the drawable passes that test.
    enum VisibilityBit : uint8_t {
        VISIBLE_FRUSTUM = 1 << 0,
        VISIBLE_LOD     = 1 << 1,
    };

    class InstanceRegistry
    {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static InstanceRegistry& get() noexcept;

        InstanceRegistry();
        ~InstanceRegistry();

        void clear();
        void prepare();

        void beginFrame();
        void endFrame();

        // @return ref to buffer
        util::BufferReference allocate(size_t count);
        // @return null ref
        util::BufferReference release(const util::BufferReference ref);

        std::span<const render::DrawableInfo> getRange(
            const util::BufferReference ref) const noexcept;

        std::span<render::DrawableInfo> modifyRange(
            const util::BufferReference ref) noexcept;

        // Compute per-drawable visibility (frustum + LOD distance) once for the
        // given camera. Result is cached in m_visible (VisibilityBit flags) and
        // read via getVisibleRange() by every pass that builds batches for this
        // camera, instead of re-testing per pass.
        // @param frustumEnabled if false, VISIBLE_FRUSTUM is set for all
        // @param lodEnabled if false, VISIBLE_LOD is set for all
        void cullFrustum(
            const Frustum& frustum,
            const glm::vec3& cameraPos,
            bool frustumEnabled,
            bool lodEnabled,
            uint32_t parallelLimit) noexcept;

        // Visibility flags for a drawable range, aligned with getRange(ref).
        // Returns an empty span when no valid cull covers the range (callers
        // treat empty as "frustum cull unavailable" => draw everything).
        std::span<const uint8_t> getVisibleRange(
            const util::BufferReference ref) const noexcept;

        // NOTE KI debug aid: verify the cached cull matches the frustum a pass is
        // about to render with (catches a batch-building root that forgot to cull)
        bool cullSignatureMatches(const Frustum& frustum) const noexcept;

        void markDirtyAll() noexcept;
        void markDirty(
            const util::BufferReference ref) noexcept;

        void prepareInstances(
            const util::BufferReference ref) noexcept;
        void updateInstances(
            const util::BufferReference ref) noexcept;

        //void updateInstances();

        // Upload to GPU (call once per frame after updateTransforms)
        void upload();

        void upload(
            const util::BufferReference ref);

        size_t getDrawableCount() const { return m_drawables.size(); }

    private:
        void resizeBuffer(size_t totalCount);

    private:
        bool m_debug{ false };

        std::vector<DrawableInfo> m_drawables;

        // Per-drawable frustum visibility cache (parallel to m_drawables),
        // recomputed once per camera per frame by cullFrustum()
        std::vector<uint8_t> m_visible;
        std::array<glm::vec4, 6> m_cullSignature{};
        bool m_cullValid{ false };

        util::SlotAllocator m_slotAllocator;
        util::DirtySet<util::BufferReference> m_dirtySlots;

        std::vector<render::InstanceSSBO> m_instances;

        kigl::GLBuffer m_ssbo{ "instances" };
        kigl::GLFence m_fence{ "instances_fence" };

        bool m_needUpload{ false };
        size_t m_uploadedCount{ 0 };
    };
}
