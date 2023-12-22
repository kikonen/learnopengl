#include "MaterialVBO.h"

#include <fmt/format.h>

#include "Program.h"
#include "ModelMesh.h"

namespace {
    int instancedCount = 0;
    int vertecedCount = 0;

    std::unique_ptr<Material> NULL_MATERIAL;

    Material* getNullMaterial()
    {
        if (!NULL_MATERIAL) {
            NULL_MATERIAL = std::make_unique<Material>();
        }
        return NULL_MATERIAL.get();
    }
}

MaterialVBO::MaterialVBO(MaterialVBO&& o)
    : m_bufferIndex{ o.m_bufferIndex },
    m_materials{ std::move(o.m_materials) },
    m_indeces { std::move(o.m_indeces) },
    m_prepared { o.m_prepared },
    m_defaultMaterial{ std::move(o.m_defaultMaterial) },
    m_useDefaultMaterial{ o.m_useDefaultMaterial },
    m_forceDefaultMaterial{ o.m_forceDefaultMaterial }
{
    o.m_bufferIndex = 0;
}

void MaterialVBO::setMaterials(const std::vector<Material>& materials)
{
    m_materials = materials;
    if (m_useDefaultMaterial) {
        m_defaultMaterial->m_default = true;
        m_defaultMaterial->m_id = Material::DEFAULT_ID;

        if (m_forceDefaultMaterial) {
            m_materials.clear();
        }

        for (auto& material : m_materials) {
            if (material.m_default) {
                material = *m_defaultMaterial;
            }
        }

        if (m_materials.empty()) {
            m_materials.push_back(*m_defaultMaterial);
        }
    }
}

const std::vector<Material>& MaterialVBO::getMaterials() const noexcept
{
    return m_materials;
}

const Material& MaterialVBO::getFirst() const noexcept
{
    if (m_materials.empty()) return *NULL_MATERIAL;
    return m_materials[0];
}

void MaterialVBO::setDefaultMaterial(
    const Material& material,
    bool useDefaultMaterial,
    bool forceDefaultMaterial
)
{
    m_defaultMaterial = std::make_unique<Material>(material);
    m_useDefaultMaterial = useDefaultMaterial;
    m_forceDefaultMaterial = forceDefaultMaterial;
}

Material* MaterialVBO::getDefaultMaterial() const
{
    return m_defaultMaterial.get();
}
