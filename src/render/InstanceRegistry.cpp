#include "InstanceRegistry.h"

#include "util/thread.h"

#include "asset/Assets.h"

#include "render/InstanceSSBO.h"

#include "shader/SSBO.h"

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

        m_instances.clear();

        m_drawables.reserve(BLOCK_SIZE);
        m_slotAllocator.reserve(BLOCK_SIZE);
        m_dirtySlots.reserve(BLOCK_SIZE);
        m_instances.reserve(BLOCK_SIZE);

        m_uploadedCount = 0;

        // NULL entry
        allocate(1);
    }

    void InstanceRegistry::prepare()
    {
        const auto& assets = Assets::get();

        // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
        GLuint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        m_ssbo.createEmpty(BLOCK_SIZE * sizeof(InstanceSSBO), flags);
        m_ssbo.map(flags);

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

        return { offset, count };
    }

    util::BufferReference InstanceRegistry::release(util::BufferReference ref)
    {
        ASSERT_RT();

        if (!m_slotAllocator.release(ref)) return {};

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
        util::BufferReference ref) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<DrawableInfo>{};

        return std::span{ m_drawables }.subspan(ref.offset, ref.size);
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
        util::BufferReference ref) noexcept
    {
        m_dirtySlots.markDirty(ref);
    }

    void InstanceRegistry::prepareInstances(util::BufferReference ref) noexcept
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

    void InstanceRegistry::updateInstances(util::BufferReference ref) noexcept
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

        m_dirtySlots.clear();
        m_uploadedCount = totalCount;
        m_needUpload = false;
    }

    void InstanceRegistry::upload(util::BufferReference ref)
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

        GLuint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        m_ssbo.map(flags);

        m_ssbo.bindSSBO(SSBO_INSTANCES);
    }
}
