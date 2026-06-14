#include "InstanceRegistry.h"

#include <algorithm>
#include <execution>

#include "util/thread.h"

#include "asset/Assets.h"
#include "asset/Frustum.h"

#include "render/InstanceSSBO.h"

#include "shader/SSBO.h"

#include "kigl/GLBuffer.h"

namespace
{
    constexpr size_t BLOCK_SIZE = 1000;
    constexpr size_t MAX_BLOCK_COUNT = 500;
    constexpr size_t MAX_COUNT = BLOCK_SIZE * MAX_BLOCK_COUNT;
    constexpr size_t MAX_INSTANCE_BUFFERS = 2;

    static render::InstanceRegistry* s_registry{ nullptr };
}

namespace render
{
    void InstanceRegistry::init() noexcept
    {
        assert(!s_registry);
        s_registry = new InstanceRegistry();
    }

    void InstanceRegistry::release() noexcept
    {
        auto* s = s_registry;
        s_registry = nullptr;
        delete s;
    }

    InstanceRegistry& InstanceRegistry::get() noexcept
    {
        assert(s_registry);
        return *s_registry;
    }

    InstanceRegistry::InstanceRegistry()
    {
        clear();
    }

    InstanceRegistry::~InstanceRegistry() = default;

    void InstanceRegistry::clear()
    {
        m_drawables.clear();
        m_slotAllocator.clear();
        m_dirtySlots.clear();

        m_cullValid = false;

        m_instances.clear();

        m_drawables.reserve(BLOCK_SIZE);
        m_slotAllocator.reserve(BLOCK_SIZE);
        m_dirtySlots.reserve(BLOCK_SIZE);
        m_instances.reserve(BLOCK_SIZE);

        m_visibility.clear();
        m_visibility.reserve(BLOCK_SIZE);

        m_uploadedCount = 0;

        // NULL entry
        allocate(1);
    }

    void InstanceRegistry::prepare()
    {
        // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
        m_ssbo.createEmpty(BLOCK_SIZE * sizeof(InstanceSSBO), kigl::getBufferStorageFlags());
        m_ssbo.map(kigl::getBufferMapFlags());

        m_ssbo.bindSSBO(SSBO_INSTANCES);

        clear();
    }

    util::BufferReference InstanceRegistry::allocate(size_t count)
    {
        ASSERT_RT();

        if (count == 0) return {};

        uint32_t offset;
        {
            int32_t freeOffset = m_slotAllocator.tryAllocate(static_cast<uint32_t>(count));
            if (freeOffset >= 0) {
                offset = static_cast<uint32_t>(freeOffset);
            }
            else {
                offset = static_cast<uint32_t>(m_drawables.size());
                m_drawables.resize(m_drawables.size() + count);
                m_slotAllocator.confirmAllocation(offset, static_cast<uint32_t>(count));
            }

            markDirty({ offset, count });
        }

        m_instances.resize(m_drawables.size());

        // keep the dense visibility array index-aligned with m_drawables; grown bytes
        // default to 0 (dead) until cullFrustum/setVisibility writes them. A reused slot
        // (tryAllocate path, no growth) keeps the 0 written by release() until reculled.
        m_visibility.resize(m_drawables.size());

        return { offset, count };
    }

    util::BufferReference InstanceRegistry::release(
        const util::BufferReference ref)
    {
        ASSERT_RT();

        if (!m_slotAllocator.release(ref)) return {};

        // NOTE KI invalidate the freed drawables so a not-yet-reused slot is skipped
        // (entityIndex == 0) and never drawn (incl. by a future per-drawable sweep);
        // on reuse, allocate -> populate -> upload overwrites these.
        for (uint32_t i = 0; i < ref.size; i++) {
            auto& drawable = m_drawables[ref.offset + i];
            drawable.entityIndex = 0;
            drawable.drawOptions.m_type = backend::DrawOptions::Type::none;
            // clear VISIBLE_ALIVE (and all bits) so the dense reject scan skips this slot
            // without an entityIndex read; reuse re-sets it at the next cull.
            m_visibility[ref.offset + i] = 0;
        }

        return {};
    }

    std::span<const DrawableInfo> InstanceRegistry::getRange(
        const util::BufferReference ref) const noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<const DrawableInfo>{};

        return std::span{ m_drawables }.subspan(ref.offset, ref.size);
    }

    std::span<DrawableInfo> InstanceRegistry::modifyRange(
        const util::BufferReference ref) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<DrawableInfo>{};

        return std::span{ m_drawables }.subspan(ref.offset, ref.size);
    }

    void InstanceRegistry::cullFrustum(
        std::span<const util::BufferReference> groups,
        const Frustum& frustum,
        const glm::vec3& cameraPos,
        bool frustumEnabled,
        bool lodEnabled,
        uint32_t parallelLimit,
        const std::function<bool(const render::DrawableInfo&)>* selector) noexcept
    {
        const size_t groupCount = groups.size();

        auto* drawables = m_drawables.data();
        auto* visibility = m_visibility.data();

        // Test frustum + LOD-distance ONCE per group (all drawables in a group share the world
        // volume, so the frustum result and the eye-distance are identical), then broadcast the
        // per-camera visibility mask to the group's drawables. Only the LOD distance *band*
        // (min/maxDistance2) is per-drawable. An off-screen group skips all per-drawable LOD work.
        const auto cull = [drawables, visibility, &frustum, &cameraPos, frustumEnabled, lodEnabled, selector]
            (const util::BufferReference& group)
        {
            const auto& rep = drawables[group.offset];

            // defensive: freed groups are erased in release() before reuse, so this shouldn't hit
            if (rep.entityIndex == 0) return;

            // NOTE KI per-group noFrustum (type-level no_frustum) is never frustum-culled
            const bool inFrustum =
                !frustumEnabled || rep.m_flags.noFrustum || rep.worldVolume.isOnFrustum(frustum);

            float dist2 = 0.f;
            if (inFrustum && lodEnabled) {
                const glm::vec3 delta = rep.worldVolume.getCenter() - cameraPos;
                dist2 = glm::dot(delta, delta);
            }

            const uint32_t end = group.offset + group.size;
            for (uint32_t i = group.offset; i < end; i++) {
                const auto& d = drawables[i];

                uint8_t v = 0;
                if (inFrustum) {
                    v |= VISIBLE_FRUSTUM;
                    if (!lodEnabled || (d.minDistance2 <= dist2 && dist2 < d.maxDistance2)) {
                        v |= VISIBLE_LOD;
                    }
                }

                // dynamic node show/hide (mirrored to the drawable via node_visible event)
                if (!d.m_flags.hidden) {
                    v |= VISIBLE_SHOWN;
                }

                // base drawableSelector folded in once here (vs. per drawable per pass).
                // nullptr selector => accept-all, no indirect call.
                if (!selector || (*selector)(d)) {
                    v |= VISIBLE_SELECTED;
                }

                // group lifecycle is uniform (alloc/free as one BufferReference) and the
                // dead-rep early-return above guarantees this group is live, so mark ALIVE.
                visibility[i] = static_cast<uint8_t>(v | VISIBLE_ALIVE);
            }
        };

        if (groupCount > parallelLimit) {
            std::for_each(
                std::execution::par_unseq,
                groups.begin(), groups.end(),
                cull);
        }
        else {
            std::for_each(
                std::execution::seq,
                groups.begin(), groups.end(),
                cull);
        }

        m_cullSignature = frustum.getPlanes();
        m_cullValid = true;
    }

    bool InstanceRegistry::cullSignatureMatches(const Frustum& frustum) const noexcept
    {
        return m_cullValid && m_cullSignature == frustum.getPlanes();
    }

    void InstanceRegistry::markDirtyAll() noexcept
    {
        m_dirtySlots.clear();
        for (const auto& [ref, allocated] : m_slotAllocator.getAllocatedSlots()) {
            if (!allocated) continue;
            m_dirtySlots.markDirty(ref);
        }
    }

    void InstanceRegistry::markDirty(
        const util::BufferReference ref) noexcept
    {
        m_dirtySlots.markDirty(ref);
    }

    void InstanceRegistry::prepareInstances(
        const util::BufferReference ref) noexcept
    {
        if (ref.size == 0) return;

        for (uint32_t i = 0; i < ref.size; i++) {
            const auto drawableIndex = ref.offset + i;
            const auto& drawable = m_drawables[drawableIndex];
            auto& instance = m_instances[drawableIndex];

            instance.u_entityIndex = drawable.entityIndex;
            instance.u_materialIndex = drawable.materialIndex;
            instance.u_jointBaseIndex = drawable.jointBaseIndex;
            instance.u_data = drawable.data;
            instance.u_flags = drawable.drawOptions.m_flags;

            instance.setTransform(drawable.localTransform);
        }
        m_needUpload = true;
    }

    void InstanceRegistry::updateInstances(
        const util::BufferReference ref) noexcept
    {
        prepareInstances(ref);
        //if (ref.size == 0) return;

        //for (uint32_t i = 0; i < ref.size; i++) {
        //    const auto drawableIndex = ref.offset + i;
        //    const auto& drawable = m_drawables[drawableIndex];
        //    auto& instance = m_instances[drawableIndex];

        //    instance.setTransform(drawable.localTransform);
        //}
        //m_needUpload = true;
    }

//    void InstanceRegistry::updateInstances()
//    {
//        if (m_dirtySlots.empty()) return;
//
//        m_instances.resize(m_drawables.size());
//
//#pragma omp parallel for schedule(static, 256)
//        for (const auto& slot : m_dirtySlots) {
//            for (size_t i = 0; i < slot.size; i++) {
//                updateInstances(slot);
//            }
//        }
//
//        m_dirtySlots.clear();
//        m_needUpload = true;
//    }

    void InstanceRegistry::upload()
    {
        if (!m_needUpload) return;

        constexpr size_t sz = sizeof(InstanceSSBO);

        const size_t totalCount = m_instances.size();
        {
            resizeBuffer(totalCount);

            auto* __restrict mappedData = m_ssbo.mapped<InstanceSSBO>(0);

            std::copy(
                std::begin(m_instances),
                std::end(m_instances),
                mappedData);
        }

        m_ssbo.markUsed(totalCount * sz);

        // NOTE KI flush for explicit mode (no-op if using coherent mapping)
        m_ssbo.flushRange(0, totalCount * sz);

        // NOTE KI memory barrier to ensure instance data writes are visible
        // to GPU before rendering, even with GL_MAP_COHERENT_BIT
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        m_dirtySlots.clear();
        m_uploadedCount = totalCount;
        m_needUpload = false;
    }

    void InstanceRegistry::upload(
        const util::BufferReference ref)
    {
        constexpr size_t sz = sizeof(InstanceSSBO);

        const size_t totalCount = m_instances.size();
        {
            resizeBuffer(totalCount);

            auto* __restrict mappedData = m_ssbo.mapped<InstanceSSBO>(0);

            std::copy(
                std::begin(m_instances) + ref.begin(),
                std::begin(m_instances) + ref.end(),
                mappedData + ref.begin());
        }

        m_ssbo.markUsed(totalCount * sz);

        // NOTE KI flush for explicit mode (no-op if using coherent mapping)
        m_ssbo.flushRange(ref.offset * sz, ref.size * sz);
    }

    void InstanceRegistry::beginFrame()
    {
        m_fence.waitFence();
    }

    void InstanceRegistry::endFrame()
    {
        m_fence.setFence();
    }

    void InstanceRegistry::resizeBuffer(size_t totalCount)
    {
        constexpr auto sz = sizeof(InstanceSSBO);

        if (m_ssbo.isCreated() && m_ssbo.size() >= totalCount * sz) return;

        size_t blocks = (totalCount / BLOCK_SIZE) + 2;
        size_t bufferSize = blocks * BLOCK_SIZE * sz;

        // NOTE KI *reallocate* SSBO if needed
        m_ssbo.resizeBuffer(bufferSize, true);

        m_ssbo.map(kigl::getBufferMapFlags());

        m_ssbo.bindSSBO(SSBO_INSTANCES);
    }
}
