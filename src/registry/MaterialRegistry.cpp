#include "MaterialRegistry.h"

MaterialRegistry::MaterialRegistry(const Assets& assets)
    : assets(assets)
{
}

int MaterialRegistry::add(const Material& material)
{
    m_materials.emplace_back(material);
    return m_materials.size() - 1;
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

void MaterialRegistry::prepareMaterials()
{
    {
        int materialIndex = 0;

        for (auto& material : m_materials) {
            material.m_index = materialIndex++;
            assert(material.m_index < MATERIAL_COUNT);

            material.prepare(assets);

            for (auto& tex : material.m_textures) {
                if (!tex.texture) continue;
                tex.m_texIndex = tex.texture->m_texIndex;
            }
        }
    }

    // materials
    {
        int sz_single = sizeof(MaterialUBO);
        //int sz = sizeof(MaterialUBO);
        m_materialsUboSize = sz_single * m_materials.size();

        MaterialsUBO materialsUbo{};

        for (auto& material : m_materials) {
            materialsUbo.materials[material.m_index] = material.toUBO();
        }

        glCreateBuffers(1, &m_materialsUboId);
        glNamedBufferStorage(m_materialsUboId, m_materialsUboSize, &materialsUbo, 0);
    }
}
