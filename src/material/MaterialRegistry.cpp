#include "MaterialRegistry.h"

#include "fmt/format.h"

#include "util/Thread.h"

#include "shader/SSBO.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "material/MaterialSSBO.h"
#include "material/MaterialUpdater.h"

namespace {
    // NOTE KI int16_t
    constexpr size_t BLOCK_SIZE = 400;
    constexpr size_t MAX_BLOCK_COUNT = 1000;

    constexpr size_t MAX_COUNT = BLOCK_SIZE * MAX_BLOCK_COUNT;

    static MaterialRegistry* s_registry{ nullptr };
}

void MaterialRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new MaterialRegistry();
}

void MaterialRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

MaterialRegistry& MaterialRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

MaterialRegistry::MaterialRegistry()
{
    clear();
}

MaterialRegistry::~MaterialRegistry() = default;

void MaterialRegistry::clear()
{
    m_materials.clear();
    m_idToIndex.clear();

    m_dirtyMaterials.clear();
    m_materialMainEntries.clear();
    m_materialCustomEntries.clear();
    m_materialColdEntries.clear();
    m_updaters.clear();

    m_materials.reserve(BLOCK_SIZE);
    m_materialMainEntries.reserve(BLOCK_SIZE);
    m_materialCustomEntries.reserve(BLOCK_SIZE);
    m_materialColdEntries.reserve(BLOCK_SIZE);

    m_dirtyFlag = false;
    m_lastSize = 0;

    {
        // NOTE KI *reserve* index 0
        // => multi-material needs to do "-index" trick, does not work for zero
        Material zero = Material::createMaterial(BasicMaterial::basic);
        m_materials.emplace_back(zero);
        registerMaterial(zero);
    }

    m_ssboMain.markUsed(0);
    m_ssboCustom.markUsed(0);
    m_ssboCold.markUsed(0);
}

ki::material_index MaterialRegistry::findRegisteredIndex(ki::material_id id)
{
    std::lock_guard lock(m_lock);

    const auto& it = m_idToIndex.find(id);

    return it != m_idToIndex.end() ? it->second : 0;
}

ki::material_index MaterialRegistry::registerMaterial(Material& material)
{
    if (material.m_registeredIndex >= 0) return material.m_registeredIndex;

    std::lock_guard lock(m_lock);

    if (m_materials.size() + 1 > MAX_COUNT)
        throw std::runtime_error{ fmt::format("MAX_MATERIAL_COUNT: {}", MAX_COUNT) };

    {
        size_t size = m_materials.size() + BLOCK_SIZE + BLOCK_SIZE;
        size += BLOCK_SIZE - size % BLOCK_SIZE;
        size = std::min(size, MAX_COUNT);
        if (size > m_materials.capacity()) {
            m_materials.reserve(size);
        }
    }

    material.m_registeredIndex = static_cast<ki::material_index>(m_materials.size());
    m_materials.emplace_back(material);

    m_idToIndex.insert({ material.getId(), material.m_registeredIndex});

    m_dirtyFlag = true;

    return material.m_registeredIndex;
}

void MaterialRegistry::updateMaterial(const Material& material)
{
    // NOTE KI don't allow update of index == 0
    if (material.m_registeredIndex <= 0) return;

    std::lock_guard lock(m_lock);

    m_materials[material.m_registeredIndex] = material;
    markDirty(material.m_registeredIndex);
}

void MaterialRegistry::markDirty(ki::material_index registeredIndex)
{
    m_dirtyMaterials.push_back(registeredIndex);
    m_dirtyFlag = true;
}

void MaterialRegistry::addMaterialUpdater(std::unique_ptr<MaterialUpdater> updater)
{
    registerMaterial(*updater->m_material);

    std::lock_guard lock(m_lock);
    m_updaters.insert({ updater->m_id, std::move(updater)});
}

void MaterialRegistry::renderMaterials(const render::RenderContext& ctx)
{
    std::lock_guard lock(m_lock);
    for (auto& [id, updater] : m_updaters) {
        updater->render(ctx);
        if (updater->isNeedUpdate()) {
            for (auto& registeredIndex : updater->m_dependentMaterials) {
                markDirty(registeredIndex);
                updater->setNeedUpdate(false);
            }
        }
    }
}

void MaterialRegistry::updateRT(const UpdateContext& ctx)
{
    if (!m_dirtyFlag) return;
    std::lock_guard lock(m_lock);

    prepareMaterialUpdaters(ctx.toPrepareContext());
    prepareMaterials(ctx.toPrepareContext());

    updateMaterialBuffer();
    updateDirtyMaterialBuffer();
}

void MaterialRegistry::prepare()
{
    m_ssboMain.createEmpty(BLOCK_SIZE * sizeof(MaterialMainSSBO), GL_DYNAMIC_STORAGE_BIT);
    m_ssboCustom.createEmpty(BLOCK_SIZE * sizeof(MaterialCustomSSBO), GL_DYNAMIC_STORAGE_BIT);
    m_ssboCold.createEmpty(BLOCK_SIZE * sizeof(MaterialColdSSBO), GL_DYNAMIC_STORAGE_BIT);
}

void MaterialRegistry::bindBuffers()
{
    ASSERT_RT();

    m_ssboMain.bindSSBO(SSBO_MATERIALS_MAIN);
    m_ssboCustom.bindSSBO(SSBO_MATERIALS_CUSTOM);
    m_ssboCold.bindSSBO(SSBO_MATERIALS_COLD);
}

void MaterialRegistry::prepareMaterials(const PrepareContext& ctx)
{
    const size_t index = m_lastSize;
    const size_t totalCount = m_materials.size();

    if (index == totalCount) return;
    if (totalCount == 0) return;

    for (size_t i = index; i < totalCount; i++) {
        auto& material = m_materials[i];
        material.prepare();

        if (material.m_updaterId) {
            const auto& it = m_updaters.find(material.m_updaterId);
            if (it != m_updaters.end()) {
                auto* updater = it->second.get();
                material.m_updater = updater;
                updater->m_dependentMaterials.push_back(material.m_registeredIndex);
            }
        }
    }
}

void MaterialRegistry::prepareMaterialUpdaters(const PrepareContext& ctx)
{
    for (auto& [id, updater] : m_updaters) {
        updater->prepareRT(ctx);
    }
}

void MaterialRegistry::updateMaterialBuffer()
{
    const size_t index = m_lastSize;
    const size_t totalCount = m_materials.size();

    if (index == totalCount) return;
    if (totalCount == 0) return;

    {
        m_materialMainEntries.reserve(totalCount);
        m_materialCustomEntries.reserve(totalCount);
        m_materialColdEntries.reserve(totalCount);

        // NOTE KI update m_materialsSSBO from *index*, not *updateIndex* point
        // - otherwise entries are multiplied, and indexed incorrectly
        for (size_t i = index; i < totalCount; i++) {
            const auto& material = m_materials[i];
            auto& main = m_materialMainEntries.emplace_back();
            auto& custom = m_materialCustomEntries.emplace_back();
            auto& cold = m_materialColdEntries.emplace_back();
            material.fillSSBO(main, custom, cold);
        }
    }

    {
        constexpr size_t sz = sizeof(MaterialMainSSBO);

        size_t updateIndex = index;

        resizeBuffer();

        const size_t updateCount = totalCount - updateIndex;

        m_ssboMain.update(
            updateIndex * sz,
            updateCount * sz,
            &m_materialMainEntries[updateIndex]);

        m_ssboMain.markUsed(totalCount * sz);
    }
    {
        constexpr size_t sz = sizeof(MaterialCustomSSBO);

        size_t updateIndex = index;

        resizeBuffer();

        const size_t updateCount = totalCount - updateIndex;

        m_ssboCustom.update(
            updateIndex * sz,
            updateCount * sz,
            &m_materialCustomEntries[updateIndex]);

        m_ssboCustom.markUsed(totalCount * sz);
    }
    {
        constexpr size_t sz = sizeof(MaterialColdSSBO);

        size_t updateIndex = index;

        resizeBuffer();

        const size_t updateCount = totalCount - updateIndex;

        m_ssboCold.update(
            updateIndex * sz,
            updateCount * sz,
            &m_materialColdEntries[updateIndex]);

        m_ssboCold.markUsed(totalCount * sz);
    }

    m_lastSize = totalCount;
}

void MaterialRegistry::resizeBuffer()
{
    const size_t totalCount = m_materials.size();
    {
        constexpr size_t sz = sizeof(MaterialMainSSBO);
        if (m_ssboMain.size() >= totalCount * sz) return;
        // NOTE KI *reallocate* SSBO if needed
        m_ssboMain.resizeBuffer(m_materialMainEntries.capacity() * sz, true);
    }
    {
        constexpr size_t sz = sizeof(MaterialCustomSSBO);
        if (m_ssboCustom.size() >= totalCount * sz) return;
        // NOTE KI *reallocate* SSBO if needed
        m_ssboCustom.resizeBuffer(m_materialCustomEntries.capacity() * sz, true);
    }
    {
        constexpr size_t sz = sizeof(MaterialColdSSBO);
        if (m_ssboCold.size() >= totalCount * sz) return;
        // NOTE KI *reallocate* SSBO if needed
        m_ssboCold.resizeBuffer(m_materialColdEntries.capacity() * sz, true);
    }

    bindBuffers();
}

void MaterialRegistry::updateDirtyMaterialBuffer()
{
    // NOTE KI assuming there is only few updateable materials
    // => thus one-by-one update is fine
    // => if more, may need to specify logic to reserving
    //    specific limited range to be used for updatable materials, to avoid random access
    for (auto dirtyIndex : m_dirtyMaterials) {
        auto& main = m_materialMainEntries[dirtyIndex];
        auto& custom = m_materialCustomEntries[dirtyIndex];
        auto& cold = m_materialColdEntries[dirtyIndex];

        m_materials[dirtyIndex].fillSSBO(main, custom, cold);

        {
            constexpr size_t sz = sizeof(MaterialMainSSBO);

            m_ssboMain.update(
                dirtyIndex * sz,
                1 * sz,
                &m_materialMainEntries[dirtyIndex]);
        }
        {
            constexpr size_t sz = sizeof(MaterialCustomSSBO);

            m_ssboCustom.update(
                dirtyIndex * sz,
                1 * sz,
                &m_materialCustomEntries[dirtyIndex]);
        }
        {
            constexpr size_t sz = sizeof(MaterialColdSSBO);

            m_ssboCold.update(
                dirtyIndex * sz,
                1 * sz,
                &m_materialColdEntries[dirtyIndex]);
        }
    }
    m_dirtyMaterials.clear();
}
