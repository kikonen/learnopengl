#include "MaterialVBO.h"

#include "Shader.h"
#include "ModelMesh.h"

namespace {
#pragma pack(push, 1)
    struct MaterialEntry {
        // NOTE KI uint DOES NOT work well in vertex attrs; data gets corrupted
        // => use float
        float material;
    };
#pragma pack(pop)
}

void MaterialVBO::prepare(ModelMesh& mesh)
{
    if (m_prepared) return;
    m_prepared = true;

    m_vbo.create();

    prepareVBO(mesh);
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
        glVertexArrayBindingDivisor(vao, VBO_MATERIAL_BINDING, 0);
    }
}

void MaterialVBO::prepareVBO(ModelMesh& mesh)
{
    auto& vertices = mesh.m_vertices;
    auto& materials = mesh.m_materials;

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(MaterialEntry);
    const int sz = stride_size * vertices.size();

    MaterialEntry* buffer = (MaterialEntry*)new unsigned char[sz];
    memset(buffer, 0, sz);

    {
        MaterialEntry* vbo = buffer;
        for (int i = 0; i < vertices.size(); i++) {
            const auto& vertex = vertices[i];
            const auto& m = Material::findID(vertex.materialID, materials);

            // TODO KI should use noticeable value for missing
            // => would trigger undefined array access in render side
            vbo->material = m ? (m->m_registeredIndex) : 0;

            vbo++;
        }
    }

    assert(buffer->material >= 0 && buffer->material < MAX_MATERIAL_COUNT);
    glNamedBufferStorage(m_vbo, sz, buffer, 0);
    delete[] buffer;
}
