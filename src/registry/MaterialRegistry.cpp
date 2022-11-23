#include "MaterialRegistry.h"

MaterialRegistry::MaterialRegistry(const Assets& assets)
    : assets(assets)
{
}

MaterialRegistry::~MaterialRegistry() {
}

void MaterialRegistry::add(const Material& material)
{
    if (material.m_registeredIndex >= 0) return;
    material.m_registeredIndex = m_materials.size();
    m_materials.emplace_back(material);
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
            m_materialsUbo.materials[material.m_registeredIndex] = material.toUBO();
        }

        m_buffer.update(0, sizeof(MaterialsUBO), &m_materialsUbo);
    }

    m_updatedSize = m_materials.size();
}

void MaterialRegistry::prepare()
{
    m_buffer.create();
    m_buffer.initEmpty(sizeof(MaterialsUBO), GL_DYNAMIC_STORAGE_BIT);
}

void MaterialRegistry::bind(
    const RenderContext& ctx)
{
    m_buffer.bindUniform(UBO_MATERIALS);
}
