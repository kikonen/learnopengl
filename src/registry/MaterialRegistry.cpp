#include "MaterialRegistry.h"

#include "fmt/format.h"

#include "asset/SSBO.h"
#include "asset/MaterialVBO.h"
#include "asset/MaterialUBO.h"
#include "asset/MaterialSSBO.h"
#include "asset/MaterialIndex.h"

namespace {
    constexpr size_t MATERIAL_BLOCK_SIZE = 100;
    constexpr size_t MATERIAL_BLOCK_COUNT = 100;

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
    m_materials.reserve(MATERIAL_BLOCK_SIZE);
    m_materialsSSBO.reserve(MATERIAL_BLOCK_SIZE);
    m_indeces.reserve(INDEX_BLOCK_SIZE);
}

MaterialRegistry::~MaterialRegistry() {
}

void MaterialRegistry::add(const Material& material)
{
    if (material.m_registeredIndex >= 0) return;

    if (m_materials.size() >= MAX_MATERIAL_COUNT)
        throw std::runtime_error{ fmt::format("MAX_MATERIAL_COUNT: {}", MAX_MATERIAL_COUNT) };

    if (m_materials.size() % MATERIAL_BLOCK_SIZE == 0) {
        size_t size = m_materials.size() + MATERIAL_BLOCK_SIZE * 2;
        size = std::max(size, MAX_MATERIAL_COUNT);
        m_materials.reserve(size);
    }

    material.m_registeredIndex = m_materials.size();
    m_materials.emplace_back(material);
}

void MaterialRegistry::registerMaterialVBO(MaterialVBO& materialVBO)
{
    assert(!materialVBO.m_indeces.empty());

    const size_t count = materialVBO.m_indeces.size();
    const size_t index = m_indeces.size();
    const size_t offset = index * sizeof(MaterialIndex);

    if (index + count > MAX_INDEX_COUNT)
        throw std::runtime_error{ fmt::format("MAX_INDEX_COUNT: {}", MAX_INDEX_COUNT) };

    {
        size_t size = m_indeces.size() + std::max(INDEX_BLOCK_SIZE, count) + INDEX_BLOCK_SIZE;
        size = std::min(size, MAX_INDEX_COUNT);
        m_indeces.reserve(size);
    }

    materialVBO.m_bufferIndex = index;
    materialVBO.m_buffer = &m_indexBuffer;

    m_indeces.insert(
        m_indeces.end(),
        materialVBO.m_indeces.begin(),
        materialVBO.m_indeces.end());
}

Material* MaterialRegistry::find(
    const std::string& name)
{
    const auto& it = std::find_if(
        m_materials.begin(),
        m_materials.end(),
        [&name](Material& m) { return m.m_name == name; });
    return it != m_materials.end() ? &(*it) : nullptr;
}

Material* MaterialRegistry::findID(
    const int objectID)
{
    const auto& it = std::find_if(
        m_materials.begin(),
        m_materials.end(),
        [objectID](Material& m) { return m.m_objectID == objectID; });
    return it != m_materials.end() ? &(*it) : nullptr;
}

void MaterialRegistry::update(const UpdateContext& ctx)
{
    updateMaterialBuffer();
    updateIndexBuffer();
}

void MaterialRegistry::prepare()
{
    m_ssbo.createEmpty(MATERIAL_BLOCK_SIZE * sizeof(MaterialSSBO), GL_DYNAMIC_STORAGE_BIT);
    m_indexBuffer.createEmpty(INDEX_BLOCK_SIZE * sizeof(MaterialIndex), GL_DYNAMIC_STORAGE_BIT);
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
        for (size_t i = index; i < totalCount; i++) {
            auto& material = m_materials[i];

            material.prepare(m_assets);

            for (auto& tex : material.m_textures) {
                if (!tex.texture) continue;
                tex.m_texIndex = tex.texture->m_texIndex;
            }
        }
    }

    {
        constexpr size_t sz = sizeof(MaterialSSBO);

        int updateIndex = index;

        // NOTE KI *reallocate* SSBO if needed
        if (m_ssbo.m_size < totalCount * sz) {
            m_ssbo.resizeBuffer(m_materials.capacity() * sz);
            updateIndex = 0;
        }

        const int updateCount = totalCount - updateIndex;

        for (size_t i = updateIndex; i < totalCount; i++) {
            const auto& material = m_materials[i];
            m_materialsSSBO.emplace_back(material.toSSBO());
        }

        m_ssbo.update(
            updateIndex * sz,
            updateCount * sz,
            &m_materialsSSBO[updateIndex]);
    }

    m_lastMaterialSize = totalCount;
}

void MaterialRegistry::updateIndexBuffer()
{
    const size_t index = m_lastMaterialSize;
    const size_t totalCount = m_indeces.size();

    if (index == totalCount) return;
    if (totalCount == 0) return;

    {
        constexpr size_t sz = sizeof(MaterialIndex);
        int updateIndex = index;

        // NOTE KI *reallocate* SSBO if needed
        if (m_indexBuffer.m_size < totalCount * sz) {
            m_indexBuffer.resizeBuffer(m_indeces.capacity() * sz);
            updateIndex = 0;
        }

        const int updateCount = totalCount - index;

        m_indexBuffer.update(
            updateIndex * sz,
            updateCount * sz,
            &m_indeces[updateIndex]);
    }

    m_lastIndexSize = totalCount;
}
