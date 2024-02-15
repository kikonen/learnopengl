#include "MaterialSet.h"

#include <fmt/format.h>

namespace {
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

namespace mesh {
    MaterialSet::MaterialSet(MaterialSet&& o)
        : m_bufferIndex{ o.m_bufferIndex },
        m_materials{ std::move(o.m_materials) },
        m_indeces{ std::move(o.m_indeces) },
        m_prepared{ o.m_prepared },
        m_defaultMaterial{ std::move(o.m_defaultMaterial) },
        m_useDefaultMaterial{ o.m_useDefaultMaterial },
        m_forceDefaultMaterial{ o.m_forceDefaultMaterial }
    {
        o.m_bufferIndex = 0;
    }

    MaterialSet& MaterialSet::operator=(MaterialSet&& o)
    {
        m_bufferIndex = o.m_bufferIndex;
        m_materials = std::move(o.m_materials);
        m_indeces = std::move(o.m_indeces);
        m_prepared = o.m_prepared;
        m_defaultMaterial = std::move(o.m_defaultMaterial);
        m_useDefaultMaterial = o.m_useDefaultMaterial;
        m_forceDefaultMaterial = o.m_forceDefaultMaterial;

        o.m_bufferIndex = 0;

        return *this;
    }

    MaterialSet& MaterialSet::operator=(const MaterialSet& o)
    {
        if (&o == this) return *this;

        m_bufferIndex = o.m_bufferIndex;
        m_materials = std::move(o.m_materials);
        m_indeces = std::move(o.m_indeces);
        m_prepared = o.m_prepared;

        if (o.m_defaultMaterial) {
            m_defaultMaterial = std::make_unique<Material>();
            *m_defaultMaterial = *o.m_defaultMaterial;
        }
        else {
            m_defaultMaterial.reset();
        }

        m_useDefaultMaterial = o.m_useDefaultMaterial;
        m_forceDefaultMaterial = o.m_forceDefaultMaterial;

        return *this;
    }

    MaterialSet::~MaterialSet() = default;

    void MaterialSet::setMaterials(const std::vector<Material>& materials)
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

    const Material& MaterialSet::getFirst() const noexcept
    {
        if (m_materials.empty()) return *NULL_MATERIAL;
        return m_materials[0];
    }

    void MaterialSet::setDefaultMaterial(
        const Material& material,
        bool useDefaultMaterial,
        bool forceDefaultMaterial
    )
    {
        m_defaultMaterial = std::make_unique<Material>();
        *m_defaultMaterial = material;
        m_useDefaultMaterial = useDefaultMaterial;
        m_forceDefaultMaterial = forceDefaultMaterial;
    }

    Material* MaterialSet::getDefaultMaterial() const
    {
        return m_defaultMaterial.get();
    }

    int MaterialSet::getMaterialIndex() const noexcept
    {
        if (isSingle()) {
            // NOTE KI *NO* indeces if single material
            return m_materials.empty() ? 0 : getFirst().m_registeredIndex;
        }
        // NOTE KI special trick; -1 to indicate "multi material" index
        return -static_cast<int>(m_bufferIndex);
    }
}
