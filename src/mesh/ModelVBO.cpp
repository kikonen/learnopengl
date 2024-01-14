#include "ModelVBO.h"

#include <algorithm>

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
        prepareNormal(mesh.m_vertices);
        prepareTexture(mesh.m_vertices);
        prepareIndex(mesh.m_indeces);
    }

    void ModelVBO::clear()
    {
        m_positionEntries.clear();
        m_normalEntries.clear();
        m_textureEntries.clear();
        m_indexEntries.clear();
    }

    AABB ModelVBO::calculateAABB() const noexcept
    {
        glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

        for (auto&& vertex : m_positionEntries)
        {
            minAABB.x = std::min(minAABB.x, vertex.pos.x);
            minAABB.y = std::min(minAABB.y, vertex.pos.y);
            minAABB.z = std::min(minAABB.z, vertex.pos.z);

            maxAABB.x = std::max(maxAABB.x, vertex.pos.x);
            maxAABB.y = std::max(maxAABB.y, vertex.pos.y);
            maxAABB.z = std::max(maxAABB.z, vertex.pos.z);
        }

        return { minAABB, maxAABB, false };
    }

    void ModelVBO::preparePosition(
        const std::vector<Vertex>& positions)
    {
        // https://paroj.github.io/gltut/Basic%20Optimization.html
        m_positionEntries.reserve(positions.size());

        for (const auto& vertex : positions) {
            m_positionEntries.emplace_back(vertex.pos);
        }
    }

    void ModelVBO::prepareNormal(
        const std::vector<Vertex>& vertices)
    {
        // https://paroj.github.io/gltut/Basic%20Optimization.html
        m_normalEntries.reserve(vertices.size());

        for (const auto& vertex : vertices) {
            m_normalEntries.emplace_back(vertex.normal, vertex.tangent);
        }
    }

    void ModelVBO::prepareTexture(
        const std::vector<Vertex>& vertices)
    {
        // https://paroj.github.io/gltut/Basic%20Optimization.html
        m_textureEntries.reserve(vertices.size());

        for (const auto& vertex : vertices) {
            m_textureEntries.emplace_back(vertex.texture);
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
