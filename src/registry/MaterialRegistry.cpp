#include "MaterialRegistry.h"

#include "fmt/format.h"

#include "util/Thread.h"

#include "asset/SSBO.h"
#include "asset/MaterialSSBO.h"

namespace {
    constexpr size_t MATERIAL_BLOCK_SIZE = 10;
    constexpr size_t MATERIAL_BLOCK_COUNT = 1000;

    constexpr size_t MAX_MATERIAL_COUNT = MATERIAL_BLOCK_SIZE * MATERIAL_BLOCK_COUNT;

}

MaterialRegistry& MaterialRegistry::get() noexcept
{
    static MaterialRegistry s_registry;
    return s_registry;
}

MaterialRegistry::MaterialRegistry()
{
    // HACK KI reserve nax to avoid memory alloc issue main vs. worker
    m_materials.reserve(MAX_MATERIAL_COUNT);
    m_materialsSSBO.reserve(MAX_MATERIAL_COUNT);

    // NOTE KI *reserve* index 0
    // => multi-material needs to do "-index" trick, does not work for zero
    Material zero = Material::createMaterial(BasicMaterial::basic);
    registerMaterial(zero);
}

MaterialRegistry::~MaterialRegistry() = default;

void MaterialRegistry::registerMaterial(Material& material)
{
    if (material.m_registeredIndex >= 0) return;

    std::lock_guard lock(m_lock);

    const size_t count = 1;

    if (m_materials.size() + count > MAX_MATERIAL_COUNT)
        throw std::runtime_error{ fmt::format("MAX_MATERIAL_COUNT: {}", MAX_MATERIAL_COUNT) };

    {
        size_t size = m_materials.size() + std::max(MATERIAL_BLOCK_SIZE, count) + MATERIAL_BLOCK_SIZE;
        size += MATERIAL_BLOCK_SIZE - size % MATERIAL_BLOCK_SIZE;
        size = std::min(size, MAX_MATERIAL_COUNT);
        if (size > m_materials.capacity()) {
            m_materials.reserve(size);
            m_materialsSSBO.reserve(size);
        }
    }

    material.m_registeredIndex = (int)m_materials.size();
    m_materials.emplace_back(material);
}

Material* MaterialRegistry::find(
    std::string_view name)
{
    std::lock_guard lock(m_lock);

    const auto& it = std::find_if(
        m_materials.begin(),
        m_materials.end(),
        [&name](Material& m) { return m.m_name == name; });
    return it != m_materials.end() ? &(*it) : nullptr;
}

Material* MaterialRegistry::findById(
    const int id)
{
    std::lock_guard lock(m_lock);

    const auto& it = std::find_if(
        m_materials.begin(),
        m_materials.end(),
        [id](Material& m) { return m.m_id == id; });
    return it != m_materials.end() ? &(*it) : nullptr;
}

void MaterialRegistry::updateRT(const UpdateContext& ctx)
{
    //if (!m_dirty) return;
    std::lock_guard lock(m_lock);

    updateMaterialBuffer();
}

void MaterialRegistry::prepare()
{
    m_ssbo.createEmpty(MATERIAL_BLOCK_SIZE * sizeof(MaterialSSBO), GL_DYNAMIC_STORAGE_BIT);
}

void MaterialRegistry::bind(
    const RenderContext& ctx)
{
    m_ssbo.bindSSBO(SSBO_MATERIALS);
}

void MaterialRegistry::updateMaterialBuffer()
{
    const size_t index = m_lastMaterialSize;
    const size_t totalCount = m_materials.size();

    if (index == totalCount) return;
    if (totalCount == 0) return;

    {
        // NOTE KI update m_materialsSSBO from *index*, not *updateIndex* point
        // - otherwise entries are multiplied, and indexed incorrectly
        for (size_t i = index; i < totalCount; i++) {
            auto& material = m_materials[i];
            material.prepare();
            m_materialsSSBO.emplace_back(material.toSSBO());
        }
    }

    {
        constexpr size_t sz = sizeof(MaterialSSBO);

        size_t updateIndex = index;

        // NOTE KI *reallocate* SSBO if needed
        if (m_ssbo.m_size < totalCount * sz) {
            m_ssbo.resizeBuffer(m_materials.capacity() * sz);
            m_ssbo.bindSSBO(SSBO_MATERIALS);
            updateIndex = 0;
        }

        const size_t updateCount = totalCount - updateIndex;

        m_ssbo.update(
            updateIndex * sz,
            updateCount * sz,
            &m_materialsSSBO[updateIndex]);
    }

    m_lastMaterialSize = totalCount;
}
