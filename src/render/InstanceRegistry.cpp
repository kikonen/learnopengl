#include "InstanceRegistry.h"

#include "asset/Assets.h"

#include "render/InstanceSSBO.h"

#include "shader/SSBO.h"

namespace
{
    constexpr size_t INITIAL_SIZE = 128;

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

        m_drawables.reserve(INITIAL_SIZE);
        m_freeSlots.reserve(INITIAL_SIZE);
        m_dirtySlots.reserve(INITIAL_SIZE);
        m_instances.reserve(INITIAL_SIZE);

        m_uploadedCount = 0;
        //m_dirty = false;

        // NULL entry
        allocate(1);

        m_ssbo.markUsed(0);
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
        //m_useFenceDebug = false;

        //m_drawables.reserve(maxInstances);
        //m_instances.reserve(maxInstances);

        // Create persistent GPU buffer
        constexpr GLbitfield storageFlags =
            GL_MAP_WRITE_BIT |
            GL_MAP_PERSISTENT_BIT |
            GL_DYNAMIC_STORAGE_BIT;

        if (m_useMapped) {
            // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
            m_ssbo.createEmpty(INITIAL_SIZE * sizeof(InstanceSSBO), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT);
            m_ssbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
        }
        else {
            m_ssbo.createEmpty(INITIAL_SIZE * sizeof(InstanceSSBO), GL_DYNAMIC_STORAGE_BIT);
        }

        clear();
        bind();
    }

    util::BufferReference InstanceRegistry::allocate(uint32_t count)
    {
        //ASSERT_WT();

        if (count == 0) return { 0, 0 };

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

    //uint32_t InstanceRegistry::registerDrawable(const DrawableInfo& info)
    //{
    //    uint32_t index = static_cast<uint32_t>(m_drawables.size());
    //    m_drawables.push_back(info);

    //    m_dirty = true;
    //    return index;
    //}

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
            return old == ref;
        });
        if (it != m_dirtySlots.end()) return;

        m_dirtySlots.push_back(ref);
    }

    void InstanceRegistry::updateInstances()
    {
        if (m_dirtySlots.empty()) return;

        m_instances.resize(m_drawables.size());

#pragma omp parallel for schedule(static, 256)
        for (const auto& slot : m_dirtySlots) {
            for (size_t i = 0; i < slot.size; i++) {
                const auto& drawable = m_drawables[slot.offset + i];
                auto& instance = m_instances[slot.offset + i];

                // Store in row-major format (matching your InstanceSSBO)
                instance.setTransform(drawable.localTransform);
                instance.u_entityIndex = drawable.entityIndex;
                instance.u_materialIndex = drawable.materialIndex;
                instance.u_jointBaseIndex = drawable.jointBaseIndex;
                instance.u_data = drawable.data;
                instance.u_flags = drawable.drawOptions.m_flags;
            }
        }

        if (m_dirtyInstances.empty()) {
            m_dirtyInstances = m_dirtySlots;
        }
        else {
            m_dirtyInstances.push_back({ 0, static_cast<uint32_t>(m_dirtyInstances.size()) });
        }
        m_dirtySlots.clear();
    }

    void InstanceRegistry::upload()
    {
        if (m_dirtyInstances.empty()) return;

        constexpr size_t sz = sizeof(InstanceSSBO);

        // Upload only changed portion (or all if structure changed)
        const size_t uploadCount = m_instances.size();

        {
            // NOTE KI *reallocate* SSBO if needed
            if (m_ssbo.m_size < uploadCount * sz) {
                m_ssbo.resizeBuffer(m_instances.capacity() * sz, true);
                if (m_useMapped) {
                    m_ssbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
                }
                else {
                    m_ssbo.bindSSBO(SSBO_INSTANCES);
                }
            }
        }

        if (m_useMapped) {
            std::copy(
                std::begin(m_instances),
                std::begin(m_instances) + uploadCount,
                m_ssbo.mapped<InstanceSSBO>(0));
            m_ssbo.flushRange(0, uploadCount * sz);
        }
        else {
            m_ssbo.update(0, uploadCount * sz, m_instances.data());
        }

        m_ssbo.markUsed(uploadCount * sz);

        m_uploadedCount = uploadCount;
        m_dirtyInstances.clear();
    }

    void InstanceRegistry::bind()
    {
        m_ssbo.bindSSBO(SSBO_INSTANCES);
    }
}
