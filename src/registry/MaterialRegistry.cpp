#include "MaterialRegistry.h"

#include "asset/MaterialVBO.h"

namespace {
    // scene_full = 91 109
    constexpr int MAX_MATERIAL_ENTRIES = 200000;
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
    if (materialVBO.m_entries.empty()) return;

    const int sz = sizeof(MaterialEntry);
    const int index = m_materialEntries.size();

    materialVBO.m_offset = index * sz;
    materialVBO.m_vbo = &m_vbo;

    for (auto& entry : materialVBO.m_entries) {
        m_materialEntries.push_back(entry);
    }

    assert(m_materialEntries.size() <= MAX_MATERIAL_ENTRIES);

    KI_INFO_SB("material_entries: " << m_materialEntries.size());

    m_vbo.update(
        materialVBO.m_offset,
        (m_materialEntries.size() - index) * sz,
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
        for (int i = m_updatedSize; i < m_materials.size(); i++) {
            auto& material = m_materials[i];

            material.prepare(assets);

            for (auto& tex : material.m_textures) {
                if (!tex.texture) continue;
                tex.m_texIndex = tex.texture->m_texIndex;
            }
        }
    }

    {
        for (int i = m_updatedSize; i < m_materials.size(); i++) {
            auto& material = m_materials[i];
            //m_materialsUbo.materials[i] = material.toUBO();
            m_materialsSSBO.emplace_back(material.toSSBO());
        }

        //m_ubo.update(0, sizeof(MaterialsUBO), &m_materialsUbo);
        const int sz = sizeof(MaterialSSBO);
        m_ssbo.update(
            m_updatedSize * sz,
            (m_materialsSSBO.size() - m_updatedSize) * sz,
            &m_materialsSSBO[m_updatedSize]);
    }

    m_updatedSize = m_materials.size();
}

void MaterialRegistry::prepare()
{
    //m_ubo.create();
    //m_ubo.initEmpty(sizeof(MaterialsUBO), GL_DYNAMIC_STORAGE_BIT);

    {
        m_ssbo.create();
        m_ssbo.initEmpty(MAX_SSBO_MATERIALS * sizeof(MaterialSSBO), GL_DYNAMIC_STORAGE_BIT);
    }

    {
        m_vbo.create();
        m_vbo.initEmpty(MAX_MATERIAL_ENTRIES * sizeof(MaterialEntry), GL_DYNAMIC_STORAGE_BIT);
    }
}

void MaterialRegistry::bind(
    const RenderContext& ctx)
{
    //m_ubo.bindUniform(UBO_MATERIALS);

    m_ssbo.bindSSBO(SSBO_MATERIALS);
}
