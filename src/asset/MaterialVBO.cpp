#include "MaterialVBO.h"

#include <fmt/format.h>

#include "Shader.h"
#include "ModelMesh.h"
#include "MaterialEntry.h"

namespace {
    int instancedCount = 0;
    int vertecedCount = 0;
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

//void MaterialVBO::prepareVAO(
//    GLVertexArray& vao)
//{
//    if (m_singleMaterial) return;
//
//    KI_GL_CALL(glVertexArrayVertexBuffer(vao, VBO_MATERIAL_BINDING, *m_vbo, m_offset, sizeof(MaterialEntry)));
//    {
//        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);
//
//        // materialID attr
//        glVertexArrayAttribFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(MaterialEntry, materialIndex));
//
//        glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_MATERIAL_BINDING);
//
//        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
//        // https://www.khronos.org/opengl/wiki/Vertex_Specification
//        // divisor == 0 per vertex material
//        glVertexArrayBindingDivisor(vao, VBO_MATERIAL_BINDING, m_instanceDivisor);
//
//        if (m_instanceDivisor > 0) {
//            instancedCount++;
//        }
//        else {
//            vertecedCount++;
//        }
//        //std::cout << fmt::format("count={}, divisor={}, instanced={}, verteced={}", m_materials.size(), m_instanceDivisor, instancedCount, vertecedCount) << "\n";
//    }
//}
