#include "MaterialVBO.h"

#include "Shader.h"
#include "ModelMesh.h"
#include "MaterialEntry.h"

namespace {
}

void MaterialVBO::setMaterials(const std::vector<Material>& materials)
{
    m_materials = materials;
    if (m_useDefaultMaterial) {
        m_defaultMaterial.m_default = true;
        m_defaultMaterial.m_objectID = Material::DEFAULT_ID;

        if (m_forceDefaultMaterial) {
            m_materials.clear();
        }

        for (auto& material : m_materials) {
            if (material.m_default) {
                material = m_defaultMaterial;
            }
        }

        if (m_materials.empty()) {
            m_materials.push_back(m_defaultMaterial);
        }
    }
}

const std::vector<Material>& MaterialVBO::getMaterials() const
{
    return m_materials;
}

void MaterialVBO::create()
{
    if (m_vbo != -1) return;
    m_vbo.create();
}

void MaterialVBO::prepareVAO(
    GLVertexArray& vao)
{
    {
        const int sz = m_entries.size() * sizeof(MaterialEntry);
        m_vbo.init(sz, m_entries.data(), 0);
    }

    glVertexArrayVertexBuffer(vao, VBO_MATERIAL_BINDING, m_vbo, m_offset, sizeof(MaterialEntry));
    {
        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);

        // materialID attr
        glVertexArrayAttribFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(MaterialEntry, material));

        glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_MATERIAL_BINDING);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        // NOTE KI single entry for lal DOES NOT work due to logic how intanced rendering
        // and glVertexArrayBindingDivisor work (MUST seemingly match instanced count)
        glVertexArrayBindingDivisor(vao, VBO_MATERIAL_BINDING, 0);
    }
}
