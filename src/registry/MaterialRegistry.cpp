#include "MaterialRegistry.h"

#include "fmt/format.h"

#include "asset/MaterialVBO.h"

namespace {
    // scene_full = 91 109
    constexpr int MAX_MATERIAL_ENTRIES = 100000;
}

MaterialRegistry::MaterialRegistry(const Assets& assets)
    : assets(assets)
{
    m_materials.reserve(MATERIAL_COUNT);
    m_materialEntries.reserve(MAX_MATERIAL_ENTRIES);
    m_materialsSSBO.reserve(MAX_SSBO_MATERIALS);
}

MaterialRegistry::~MaterialRegistry() {
}

void MaterialRegistry::add(const Material& material)
{
    if (material.m_registeredIndex >= 0) return;
    material.m_registeredIndex = m_materials.size();
    m_materials.emplace_back(material);
}

void MaterialRegistry::registerMaterialVBO(MaterialVBO& materialVBO)
{
    assert(!materialVBO.m_entries.empty());

    const size_t count = materialVBO.m_entries.size();
    const size_t index = m_materialEntries.size();
    const size_t offset = index * sizeof(MaterialEntry);

    assert(index + count < MAX_MATERIAL_ENTRIES);

    materialVBO.m_bufferIndex = index;
    materialVBO.m_buffer = &m_entryBuffer;

    m_materialEntries.insert(
        m_materialEntries.end(),
        materialVBO.m_entries.begin(),
        materialVBO.m_entries.end());

    KI_INFO(fmt::format(
        "MATERIAL: offset={}, mesh_entries={}, total_entries={}, BUFFER_SIZE={}",
        materialVBO.m_bufferIndex, materialVBO.m_entries.size(), m_materialEntries.size(), MAX_MATERIAL_ENTRIES * sizeof(MaterialEntry)));

    m_entryBuffer.update(
        offset,
        count * sizeof(MaterialEntry),
        &m_materialEntries[index]);
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

void MaterialRegistry::update(const RenderContext& ctx)
{
    if (m_updatedSize == m_materials.size()) return;
    if (m_materials.size() == 0) return;

    {
        for (size_t i = m_updatedSize; i < m_materials.size(); i++) {
            auto& material = m_materials[i];

            material.prepare(assets);

            for (auto& tex : material.m_textures) {
                if (!tex.texture) continue;
                tex.m_texIndex = tex.texture->m_texIndex;
            }
        }
    }

    {
        for (size_t i = m_updatedSize; i < m_materials.size(); i++) {
            auto& material = m_materials[i];
            m_materialsSSBO.emplace_back(material.toSSBO());
        }

        const size_t sz = sizeof(MaterialSSBO);
        m_ssbo.update(
            m_updatedSize * sz,
            (m_materialsSSBO.size() - m_updatedSize) * sz,
            &m_materialsSSBO[m_updatedSize]);
    }

    m_updatedSize = m_materials.size();
}

void MaterialRegistry::prepare()
{
    m_ssbo.createEmpty(MAX_SSBO_MATERIALS * sizeof(MaterialSSBO), GL_DYNAMIC_STORAGE_BIT);
    m_entryBuffer.createEmpty(MAX_MATERIAL_ENTRIES * sizeof(MaterialEntry), GL_DYNAMIC_STORAGE_BIT);
}

void MaterialRegistry::bind(
    const RenderContext& ctx)
{
    m_ssbo.bindSSBO(SSBO_MATERIALS);
    m_entryBuffer.bindSSBO(SSBO_MATERIAL_INDECES);
}
