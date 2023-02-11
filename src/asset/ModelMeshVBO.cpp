#include "ModelMeshVBO.h"

#include "glm/glm.hpp"

#include "Program.h"
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

    m_single = mesh.getMaterials().size() == 1;

    prepareBuffers(mesh);
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
        auto& entry = m_vertexEntries.emplace_back();

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
