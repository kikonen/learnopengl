#include "MaterialRegistry.h"

#include "fmt/format.h"

#include "asset/SSBO.h"
#include "asset/MaterialVBO.h"
#include "asset/MaterialSSBO.h"


namespace {
    constexpr size_t MATERIAL_BLOCK_SIZE = 10;
    constexpr size_t MATERIAL_BLOCK_COUNT = 1000;

    constexpr size_t MAX_MATERIAL_COUNT = MATERIAL_BLOCK_SIZE * MATERIAL_BLOCK_COUNT;

    // scene_full = 91 109
    constexpr size_t INDEX_BLOCK_SIZE = 1000;
    constexpr size_t INDEX_BLOCK_COUNT = 500;

    constexpr size_t MAX_INDEX_COUNT = INDEX_BLOCK_SIZE * INDEX_BLOCK_COUNT;
}

MaterialRegistry::MaterialRegistry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
{
    // HACK KI reserve nax to avoid memory alloc issue main vs. worker
    m_materials.reserve(MAX_MATERIAL_COUNT);
    m_materialsSSBO.reserve(MAX_MATERIAL_COUNT);
    m_indeces.reserve(MAX_INDEX_COUNT);

    // NOTE KI *reserve* index 0
    // => multi-material needs to do "-index" trick, does not work for zero
    m_zero = Material::createMaterial(BasicMaterial::basic);
    registerMaterial(m_zero);
    m_indeces.emplace_back(m_zero.m_registeredIndex);
}

void MaterialRegistry::registerMaterial(Material& material)
{
    if (material.m_registeredIndex >= 0) return;

    std::lock_guard<std::mutex> lock(m_lock);

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

void MaterialRegistry::registerMaterialVBO(MaterialVBO& materialVBO)
{
    // NOTE KI *NO* indeces if single material
    if (materialVBO.isSingle()) return;

    std::lock_guard<std::mutex> lock(m_lock);

    const size_t count = materialVBO.m_indeces.size();
    const size_t index = m_indeces.size();
    const size_t offset = index * sizeof(GLuint);

    if (index + count > MAX_INDEX_COUNT)
        throw std::runtime_error{ fmt::format("MAX_INDEX_COUNT: {}", MAX_INDEX_COUNT) };

    {
        size_t size = m_indeces.size() + std::max(INDEX_BLOCK_SIZE, count) + INDEX_BLOCK_SIZE;
        size += INDEX_BLOCK_SIZE - size % INDEX_BLOCK_SIZE;
        size = std::min(size, MAX_INDEX_COUNT);
        m_indeces.reserve(size);
    }

    materialVBO.m_bufferIndex = index;
    //materialVBO.m_buffer = &m_indexBuffer;

    m_indeces.insert(
        m_indeces.end(),
        materialVBO.m_indeces.begin(),
        materialVBO.m_indeces.end());
}

Material* MaterialRegistry::find(
    std::string_view name)
{
    std::lock_guard<std::mutex> lock(m_lock);

    const auto& it = std::find_if(
        m_materials.begin(),
        m_materials.end(),
        [&name](Material& m) { return m.m_name == name; });
    return it != m_materials.end() ? &(*it) : nullptr;
}

Material* MaterialRegistry::findById(
    const int id)
{
    std::lock_guard<std::mutex> lock(m_lock);

    const auto& it = std::find_if(
        m_materials.begin(),
        m_materials.end(),
        [id](Material& m) { return m.m_id == id; });
    return it != m_materials.end() ? &(*it) : nullptr;
}

void MaterialRegistry::updateRT(const UpdateViewContext& ctx)
{
    //if (!m_dirty) return;
    std::lock_guard<std::mutex> lock(m_lock);

    updateMaterialBuffer();
    updateIndexBuffer();
}

void MaterialRegistry::prepare()
{
    m_ssbo.createEmpty(MATERIAL_BLOCK_SIZE * sizeof(MaterialSSBO), GL_DYNAMIC_STORAGE_BIT);
    m_indexBuffer.createEmpty(INDEX_BLOCK_SIZE * sizeof(GLuint), GL_DYNAMIC_STORAGE_BIT);
}

void MaterialRegistry::bind(
    const RenderContext& ctx)
{
    m_ssbo.bindSSBO(SSBO_MATERIALS);
    m_indexBuffer.bindSSBO(SSBO_MATERIAL_INDECES);
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
            material.prepare(m_assets);
            m_materialsSSBO.emplace_back(material.toSSBO());
        }
    }

    {
        constexpr size_t sz = sizeof(MaterialSSBO);

        size_t updateIndex = index;

        // NOTE KI *reallocate* SSBO if needed
        if (m_ssbo.m_size < totalCount * sz) {
            m_ssbo.resizeBuffer(m_materials.capacity() * sz);
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

void MaterialRegistry::updateIndexBuffer()
{
    const size_t index = m_lastIndexSize;
    const size_t totalCount = m_indeces.size();

    if (index == totalCount) return;
    if (totalCount == 0) return;

    {
        constexpr size_t sz = sizeof(GLuint);
        size_t updateIndex = index;

        // NOTE KI *reallocate* SSBO if needed
        if (m_indexBuffer.m_size < totalCount * sz) {
            m_indexBuffer.resizeBuffer(m_indeces.capacity() * sz);
            updateIndex = 0;
        }

        const size_t updateCount = totalCount - updateIndex;

        m_indexBuffer.update(
            updateIndex * sz,
            updateCount * sz,
            &m_indeces[updateIndex]);
    }

    m_lastIndexSize = totalCount;
}
