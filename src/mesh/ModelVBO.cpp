#include "ModelVBO.h"

#include "glm/glm.hpp"

#include "asset/Program.h"

#include "mesh/ModelMesh.h"


namespace mesh {
    ModelVBO::ModelVBO()
    {

    }

    ModelVBO::~ModelVBO()
    {
    }

    void ModelVBO::prepare(ModelMesh& mesh)
    {
        if (m_prepared) return;
        m_prepared = true;

        prepareBuffers(mesh);
    }

    void ModelVBO::prepareBuffers(
        ModelMesh& mesh)
    {
        prepareVertex(mesh);
        prepareIndex(mesh);
    }

    void ModelVBO::prepareVertex(
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

            //posEntry.pos = p;
            //entry.normal = n;
            //entry.tangent = tan;
            //entry.texCoord = t;
        }
    }

    void ModelVBO::prepareIndex(
        ModelMesh& mesh)
    {
        m_indexEntries.reserve(mesh.m_tris.size());

        for (const auto& vi : mesh.m_tris) {
            m_indexEntries.push_back(vi);
        }
    }
}
