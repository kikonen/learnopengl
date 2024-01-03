#include "ModelVBO.h"

#include "glm/glm.hpp"

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

        preparePosition(mesh.m_vertices);
        prepareVertex(mesh.m_vertices);
        prepareIndex(mesh.m_tris);
    }

    void ModelVBO::preparePosition(
        const std::vector<Vertex>& positions)
    {
        // https://paroj.github.io/gltut/Basic%20Optimization.html

        m_positionEntries.reserve(positions.size());

        for (const auto& vertex : positions) {
            const auto& p = vertex.pos;
            m_positionEntries.emplace_back(p);
        }
    }

    void ModelVBO::prepareVertex(
        const std::vector<Vertex>& vertices)
    {
        // https://paroj.github.io/gltut/Basic%20Optimization.html

        m_vertexEntries.reserve(vertices.size());

        for (const auto& vertex : vertices) {
            const auto& n = vertex.normal;
            const auto& tan = vertex.tangent;
            const auto& t = vertex.texture;

            auto& entry = m_vertexEntries.emplace_back(n, tan, t);
        }
    }

    void ModelVBO::prepareIndex(
        std::vector<glm::uvec3> indeces)
    {
        m_indexEntries.reserve(indeces.size());

        for (const auto& vi : indeces) {
            m_indexEntries.push_back(vi);
        }
    }
}
