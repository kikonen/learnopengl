#include "ModelMeshVBO.h"

#include "glm/glm.hpp"

#include "Shader.h"
#include "ModelMesh.h"

namespace {
#pragma pack(push, 1)
    struct TexVBO {
        glm::vec3 pos;
        ki::VEC10 normal;
        ki::VEC10 tangent;
        ki::UV16 texCoords;
    };

    struct MaterialVBO {
        // NOTE KI uint DOES NOT work well in vertex attrs; data gets corrupted
        // => use float
        float material;
    };
#pragma pack(pop)
}

void ModelMeshVBO::prepare(ModelMesh& mesh)
{
    if (m_prepared) return;
    m_prepared = true;

    m_vbo.create();

    prepareVBO(mesh);
}

void ModelMeshVBO::prepareVAO(GLVertexArray& vao)
{
    glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, m_vbo, 0, sizeof(TexVBO));
    {
        glEnableVertexArrayAttrib(vao, ATTR_POS);
        glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
        glEnableVertexArrayAttrib(vao, ATTR_TANGENT);
        glEnableVertexArrayAttrib(vao, ATTR_TEX);

        // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        //
        // vertex attr
        glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(TexVBO, pos));

        // normal attr
        glVertexArrayAttribFormat(vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(TexVBO, normal));

        // tangent attr
        glVertexArrayAttribFormat(vao, ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(TexVBO, tangent));

        // texture attr
        glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(TexVBO, texCoords));

        glVertexArrayAttribBinding(vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_NORMAL, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TANGENT, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TEX, VBO_VERTEX_BINDING);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_VERTEX_BINDING, 0);
    }
}

void ModelMeshVBO::prepareVBO(ModelMesh& mesh)
{
    auto vertices = mesh.m_vertices;

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(TexVBO);
    const int sz = stride_size * vertices.size();

    TexVBO* buffer = (TexVBO*)new unsigned char[sz];
    memset(buffer, 0, sz);

    {
        TexVBO* vbo = buffer;
        for (int i = 0; i < vertices.size(); i++) {
            const auto& vertex = vertices[i];
            const auto& p = vertex.pos;
            const auto& n = vertex.normal;
            const auto& tan = vertex.tangent;
            const auto& t = vertex.texture;

            vbo->pos.x = p.x;
            vbo->pos.y = p.y;
            vbo->pos.z = p.z;

            vbo->normal.x = (int)(n.x * ki::SCALE_VEC10);
            vbo->normal.y = (int)(n.y * ki::SCALE_VEC10);
            vbo->normal.z = (int)(n.z * ki::SCALE_VEC10);

            vbo->tangent.x = (int)(tan.x * ki::SCALE_VEC10);
            vbo->tangent.y = (int)(tan.y * ki::SCALE_VEC10);
            vbo->tangent.z = (int)(tan.z * ki::SCALE_VEC10);

            vbo->texCoords.u = (int)(t.x * ki::SCALE_UV16);
            vbo->texCoords.v = (int)(t.y * ki::SCALE_UV16);

            vbo++;
        }
    }

    glNamedBufferStorage(m_vbo, sz, buffer, 0);
    delete[] buffer;
}
