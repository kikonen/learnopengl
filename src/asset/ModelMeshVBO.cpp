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
        const auto& p = vertex.pos;
        const auto& n = vertex.normal;
        const auto& tan = vertex.tangent;
        const auto& t = vertex.texture;

        auto& posEntry = m_positionEntries.emplace_back(p);
        auto& entry = m_vertexEntries.emplace_back(n, tan, t);


        posEntry.pos = p;
        entry.normal = n;
        entry.tangent = tan;
        entry.texCoord = t;
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
