#include "MaterialVBO.h"

#include "Shader.h"
#include "ModelMesh.h"

#include "registry/MaterialEntry.h"

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

void MaterialVBO::prepare(
    ModelMesh& mesh)
{
    if (m_prepared) return;
    m_prepared = true;

    prepareVBO(mesh.m_vertices);
}

void MaterialVBO::prepareVAO(GLVertexArray& vao)
{
    glVertexArrayVertexBuffer(vao, VBO_MATERIAL_BINDING, m_vbo, 0, sizeof(MaterialEntry));
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

void MaterialVBO::prepareVBO(
    std::vector<Vertex>& vertices)
{
    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(MaterialEntry);
    const int sz = stride_size * vertices.size();

    MaterialEntry* buffer = (MaterialEntry*)new unsigned char[sz];
    memset(buffer, 0, sz);

    {
        MaterialEntry* vbo = buffer;
        for (int i = 0; i < vertices.size(); i++) {
            const auto& vertex = vertices[i];
            auto* m = Material::findID(vertex.materialID, m_materials);

            if (m_useDefaultMaterial) {
                if (m_forceDefaultMaterial) {
                    m = &m_materials[0];
                }
            }

            // TODO KI should use noticeable value for missing
            // => would trigger undefined array access in render side
            vbo->material = m ? m->m_registeredIndex : Material::DEFAULT_ID;

            assert(vbo->material >= 0 && vbo->material < MAX_MATERIAL_COUNT);

            vbo++;
        }
    }

    glNamedBufferStorage(m_vbo, sz, buffer, 0);
    delete[] buffer;
}
