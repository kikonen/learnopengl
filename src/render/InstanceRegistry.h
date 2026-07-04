#pragma once

#include <vector>
#include <span>
#include <array>
#include <functional>

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
        VISIBLE_SHOWN   = 1 << 2,   // not hidden (node visible)
        // slot is live. Folded into the visibility byte so the reject scan needs no
        // entityIndex (DrawableInfo) read. Set by cullFrustum/setVisibility, cleared in release().
        VISIBLE_ALIVE   = 1 << 3,
        // base drawableSelector result, folded into the byte by cullFrustum so the
        // per-pass sweep can drop the nested std::function call. Deliberately OUTSIDE
        // VISIBLE_ALL: passes that reuse another root's cull (highlight/wireframe
        // selection) must NOT be affected by it, so it is opt-in via a per-sweep
        // require-mask (VISIBLE_ALL_SELECTED), never the global reject test.
        VISIBLE_SELECTED = 1 << 4,
        // drawable is rendered only when all bits — incl. liveness — are set
        VISIBLE_ALL     = VISIBLE_FRUSTUM | VISIBLE_LOD | VISIBLE_SHOWN | VISIBLE_ALIVE,
        // require-mask for passes that folded their base selector into the cull
        VISIBLE_ALL_SELECTED = VISIBLE_ALL | VISIBLE_SELECTED,
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
        void bindBuffers();

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

        // all drawables (for the per-drawable sweep — grab once, index directly)
        std::span<const render::DrawableInfo> getDrawables() const noexcept
        {
            return m_drawables;
        }

        // Hot per-pass reject test: true iff the drawable is alive AND fully visible for
        // the last cull — a single dense-array byte read, no DrawableInfo access (liveness
        // is folded into VISIBLE_ALIVE). Cold callers use this; hot sweeps hoist
        // getVisibility().data() and index it directly.
        bool isVisible(uint32_t index) const noexcept
        {
            return (m_visibility[index] & VISIBLE_ALL) == VISIBLE_ALL;
        }

        // Force a drawable's visibility mask (debug meshes that bypass cullFrustum).
        // Pass VISIBLE_ALL to mark always-visible (includes liveness via VISIBLE_ALIVE).
        void setVisibility(uint32_t index, uint8_t v) noexcept
        {
            m_visibility[index] = v;
        }

        // Dense visibility masks, parallel to getDrawables(). Hoist .data() before a hot
        // sweep and index directly — the reject test is (v & VISIBLE_ALL) == VISIBLE_ALL.
        std::span<const uint8_t> getVisibility() const noexcept
        {
            return m_visibility;
        }

        // Compute per-drawable visibility (frustum + LOD + shown + alive) for the given camera,
        // written into the dense m_visibility array (read by the draw sweep). Tests the frustum +
        // LOD-distance once per cull group (drawables in a group share a world volume) and
        // broadcasts to the group's drawables. @param groups the cull groups to process (the
        // layer's groups, or the shadow-caster subset) — owned per-layer by NodeCollection.
        // @param frustumEnabled/lodEnabled if false that axis passes for all.
        // @param selector optional per-drawable base filter folded into VISIBLE_SELECTED;
        // nullptr means accept-all (the bit is set unconditionally, no indirect call).
        void cullFrustum(
            std::span<const util::BufferReference> groups,
            const Frustum& frustum,
            const glm::vec3& cameraPos,
            bool frustumEnabled,
            bool lodEnabled,
            uint32_t parallelLimit,
            const std::function<bool(const render::DrawableInfo&)>* selector = nullptr) noexcept;

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

        // Dense per-drawable visibility (VisibilityBit mask incl. VISIBLE_ALIVE), parallel
        // to m_drawables (same index space). Written by cullFrustum each frame; read by every
        // draw sweep via isVisible()/getVisibility(). Kept out of DrawableInfo so the reject
        // scan streams 1 byte/drawable instead of dragging the struct's cache lines through memory.
        std::vector<uint8_t> m_visibility;

        // cull signature (frustum planes) of the cull that last wrote the visibility mask
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
