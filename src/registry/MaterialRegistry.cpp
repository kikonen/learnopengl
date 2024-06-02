#include "MaterialRegistry.h"

#include "fmt/format.h"

#include "util/Thread.h"

#include "asset/SSBO.h"
#include "asset/MaterialSSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10;
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

    if (m_materials.size() + count > MAX_COUNT)
        throw std::runtime_error{ fmt::format("MAX_MATERIAL_COUNT: {}", MAX_COUNT) };

    {
        size_t size = m_materials.size() + std::max(BLOCK_SIZE, count) + BLOCK_SIZE;
        size += BLOCK_SIZE - size % BLOCK_SIZE;
        size = std::min(size, MAX_COUNT);
        if (size > m_materials.capacity()) {
            m_materials.reserve(size);
        }
    }

    material.m_registeredIndex = (int)m_materials.size();
    m_materials.emplace_back(material);
}

void MaterialRegistry::updateRT(const UpdateContext& ctx)
{
    //if (!m_dirty) return;
    std::lock_guard lock(m_lock);

    updateMaterialBuffer();
}

void MaterialRegistry::prepare()
{
    m_ssbo.createEmpty(BLOCK_SIZE * sizeof(MaterialSSBO), GL_DYNAMIC_STORAGE_BIT);
    m_ssbo.bindSSBO(SSBO_MATERIALS);
}

//void MaterialRegistry::bind(
//    const RenderContext& ctx)
//{
//    m_ssbo.bindSSBO(SSBO_MATERIALS);
//}

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
            auto& material = m_materials[i];
            material.prepare();
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
