#include "MaterialRegistry.h"

#include "fmt/format.h"

#include "util/Thread.h"

#include "shader/SSBO.h"

#include "engine/UpdateContext.h"

#include "material/MaterialSSBO.h"
#include "material/MaterialUpdater.h"

namespace {
    // NOTE KI int16_t
    constexpr size_t BLOCK_SIZE = 32;
    constexpr size_t BLOCK_COUNT = 1000;

    constexpr size_t MAX_COUNT = BLOCK_SIZE * BLOCK_COUNT;
}

MaterialRegistry& MaterialRegistry::get() noexcept
{
    static MaterialRegistry s_registry;
    return s_registry;
}

MaterialRegistry::MaterialRegistry()
{
    m_materials.reserve(BLOCK_SIZE);
    m_materialEntries.reserve(BLOCK_SIZE);

    {
        // NOTE KI *reserve* index 0
        // => multi-material needs to do "-index" trick, does not work for zero
        Material zero = Material::createMaterial(BasicMaterial::basic);
        m_materials.emplace_back(zero);
        registerMaterial(zero);
    }
}

MaterialRegistry::~MaterialRegistry() = default;

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
    std::lock_guard lock(m_lock);
    m_updaters.insert({ updater->m_id, std::move(updater)});
}

void MaterialRegistry::renderMaterials(const RenderContext& ctx)
{
    std::lock_guard lock(m_lock);
    for (auto& [id, updater] : m_updaters) {
        updater->render(ctx);
        if (updater->isBeedUpdate()) {
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

    prepareMaterialUpdaters(ctx);
    prepareMaterials(ctx);

    updateMaterialBuffer();
    updateDirtyMaterialBuffer();
}

void MaterialRegistry::prepare()
{
    m_ssbo.createEmpty(BLOCK_SIZE * sizeof(MaterialSSBO), GL_DYNAMIC_STORAGE_BIT);
    m_ssbo.bindSSBO(SSBO_MATERIALS);
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
        m_materialEntries.reserve(totalCount);

        // NOTE KI update m_materialsSSBO from *index*, not *updateIndex* point
        // - otherwise entries are multiplied, and indexed incorrectly
        for (size_t i = index; i < totalCount; i++) {
            const auto& material = m_materials[i];
            m_materialEntries.emplace_back(material.toSSBO());
        }
    }

    {
        constexpr size_t sz = sizeof(MaterialSSBO);

        size_t updateIndex = index;

        // NOTE KI *reallocate* SSBO if needed
        if (m_ssbo.m_size < totalCount * sz) {
            m_ssbo.resizeBuffer(m_materialEntries.capacity() * sz);
            m_ssbo.bindSSBO(SSBO_MATERIALS);
            updateIndex = 0;
        }

        const size_t updateCount = totalCount - updateIndex;

        m_ssbo.update(
            updateIndex * sz,
            updateCount * sz,
            &m_materialEntries[updateIndex]);
    }

    m_lastSize = totalCount;
}

void MaterialRegistry::updateDirtyMaterialBuffer()
{
    // NOTE KI assuming there is only few updateable materials
    // => thus one-by-one update is fine
    // => if more, may need to specify logic to reserving
    //    specific limited range to be used for updatable materials, to avoid random access
    for (auto dirtyIndex : m_dirtyMaterials) {
        m_materialEntries[dirtyIndex] = m_materials[dirtyIndex].toSSBO();

        constexpr size_t sz = sizeof(MaterialSSBO);

        m_ssbo.update(
            dirtyIndex * sz,
            1 * sz,
            &m_materialEntries[dirtyIndex]);
    }
    m_dirtyMaterials.clear();
}
