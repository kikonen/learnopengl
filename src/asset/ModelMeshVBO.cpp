#include "ModelMeshVBO.h"

#include "glm/glm.hpp"

#include "Shader.h"
#include "ModelMesh.h"


ModelMeshVBO::ModelMeshVBO()
{

}

ModelMeshVBO::~ModelMeshVBO()
{
}

void ModelMeshVBO::prepare(ModelMesh& mesh)
{
    if (m_prepared) return;
    m_prepared = true;

    prepareBuffers(mesh);
}

void ModelMeshVBO::prepareVAO(GLVertexArray& vao)
{
    glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, *m_vbo, m_vertexOffset, sizeof(VertexEntry));
    {
        glEnableVertexArrayAttrib(vao, ATTR_POS);
        glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
        glEnableVertexArrayAttrib(vao, ATTR_TANGENT);
        glEnableVertexArrayAttrib(vao, ATTR_TEX);

        // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        //
        // vertex attr
        glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, pos));

        // normal attr
        glVertexArrayAttribFormat(vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, normal));

        // tangent attr
        glVertexArrayAttribFormat(vao, ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, tangent));

        // texture attr
        glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, texCoords));

        glVertexArrayAttribBinding(vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_NORMAL, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TANGENT, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TEX, VBO_VERTEX_BINDING);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_VERTEX_BINDING, 0);
    }

    {
        glVertexArrayElementBuffer(vao, *m_vbo);
    }
}

void ModelMeshVBO::prepareBuffers(
    ModelMesh& mesh)
{
    prepareVertex(mesh);
    prepareIndex(mesh);
}


void ModelMeshVBO::prepareVertex(
    ModelMesh& mesh)
{
    // https://paroj.github.io/gltut/Basic%20Optimization.html

    m_vertexEntries.reserve(mesh.m_vertices.size());

    for (const auto& vertex : mesh.m_vertices) {
        VertexEntry entry;

        const auto& p = vertex.pos;
        const auto& n = vertex.normal;
        const auto& tan = vertex.tangent;
        const auto& t = vertex.texture;

        entry.pos.x = p.x;
        entry.pos.y = p.y;
        entry.pos.z = p.z;

        entry.normal.x = (int)(n.x * ki::SCALE_VEC10);
        entry.normal.y = (int)(n.y * ki::SCALE_VEC10);
        entry.normal.z = (int)(n.z * ki::SCALE_VEC10);

        entry.tangent.x = (int)(tan.x * ki::SCALE_VEC10);
        entry.tangent.y = (int)(tan.y * ki::SCALE_VEC10);
        entry.tangent.z = (int)(tan.z * ki::SCALE_VEC10);

        entry.texCoords.u = (int)(t.x * ki::SCALE_UV16);
        entry.texCoords.v = (int)(t.y * ki::SCALE_UV16);

        m_vertexEntries.push_back(entry);
    }
}

void ModelMeshVBO::prepareIndex(
    ModelMesh& mesh)
{
    m_indexEntries.reserve(mesh.m_tris.size());

    for (const auto& vi : mesh.m_tris) {
        m_indexEntries.push_back(vi);
    }
}
