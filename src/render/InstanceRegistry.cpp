#include "InstanceRegistry.h"

#include "asset/Assets.h"

#include "kigl/GLSyncQueue_impl.h"

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
        m_freeSlots.clear();
        m_dirtySlots.clear();

        m_instances.clear();

        m_drawables.reserve(BLOCK_SIZE);
        m_freeSlots.reserve(BLOCK_SIZE);
        m_dirtySlots.reserve(BLOCK_SIZE);
        m_instances.reserve(BLOCK_SIZE);

        m_uploadedCount = 0;

        // NULL entry
        allocate(1);
    }

    void InstanceRegistry::prepare()
    {
        const auto& assets = Assets::get();

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useFenceDebug = assets.glUseFenceDebug;

        m_useMapped = false;
        m_useInvalidate = true;
        m_useFence = false;

        clear();
        bind();
    }

    util::BufferReference InstanceRegistry::allocate(size_t count)
    {
        //ASSERT_WT();

        if (count == 0) return {};

        uint32_t offset;
        {
            auto it = m_freeSlots.find(count);
            if (it != m_freeSlots.end() && !it->second.empty()) {
                offset = it->second[it->second.size() - 1];
                it->second.pop_back();
            }
            else {
                offset = static_cast<uint32_t>(m_drawables.size());
                m_drawables.resize(m_drawables.size() + count);
            }

            markDirty({ offset, count });
        }

        m_instances.resize(m_drawables.size());

        return { offset, count };
    }

    util::BufferReference InstanceRegistry::release(util::BufferReference ref)
    {
        if (ref.size == 0) return {};

        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return {};

        auto it = m_freeSlots.find(ref.size);
        if (it == m_freeSlots.end()) {
            m_freeSlots[ref.size] = std::vector<uint32_t>{ ref.offset };
        }
        else {
            it->second.push_back(ref.offset);
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
        util::BufferReference ref) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<DrawableInfo>{};

        return std::span{ m_drawables }.subspan(ref.offset, ref.size);
    }

    void InstanceRegistry::markDirtyAll() noexcept
    {
        m_dirtySlots.clear();
        markDirty({ 0, static_cast<uint32_t>(m_drawables.size()) });
    }

    void InstanceRegistry::markDirty(
        util::BufferReference ref) noexcept
    {
        //ASSERT_WT();
        if (ref.size == 0) return;

        const auto& it = std::find_if(
            m_dirtySlots.begin(),
            m_dirtySlots.end(),
            [&ref](const auto& old) {
            return old.contains(ref);
        });

        if (it != m_dirtySlots.end()) return;

        m_dirtySlots.push_back(ref);
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
            createInstanceBuffers(totalCount);

            auto& current = m_instanceBuffers->current();
            auto* __restrict mappedData = m_instanceBuffers->currentMapped();

            m_instanceBuffers->waitFence();
            std::copy(
                std::begin(m_instances),
                std::end(m_instances),
                mappedData);
            m_instanceBuffers->flush();
            m_instanceBuffers->bindCurrentSSBO(SSBO_INSTANCES, false, totalCount);
        }

        if (!m_instanceBuffers->setFenceIfNotSet()) {
            KI_OUT(fmt::format("DUPLICATE_FENCE"));
        }
        m_instanceBuffers->next();

        m_dirtySlots.clear();
        m_uploadedCount = totalCount;
        m_needUpload = false;
    }

    void InstanceRegistry::bind()
    {
    }

    void InstanceRegistry::createInstanceBuffers(size_t totalCount)
    {
        if (!m_instanceBuffers || m_instanceBuffers->getEntryCount() < totalCount) {
            size_t blocks = static_cast<size_t>((totalCount * 1.25f / BLOCK_SIZE) + 2);
            size_t entryCount = blocks * BLOCK_SIZE;

            m_instanceBuffers = std::make_unique<kigl::GLSyncQueue<render::InstanceSSBO>>(
                "instance",
                entryCount,
                MAX_INSTANCE_BUFFERS,
                true,
                false,
                true,
                m_useFenceDebug);
            m_instanceBuffers->prepare(1, m_debug);
        }
    }
}
