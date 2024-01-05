#include "ModelMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "kigl/kigl.h"

#include "asset/Sphere.h"

#include "mesh/ModelMaterialInit.h"
#include "mesh/MeshType.h"

#include "registry/ModelRegistry.h"


namespace {
}

namespace mesh {
    ModelMesh::ModelMesh(
        std::string_view meshName,
        std::string_view rootDir)
        : ModelMesh(meshName, rootDir, "")
    {
    }

    ModelMesh::ModelMesh(
        std::string_view meshName,
        std::string_view rootDir,
        std::string_view meshPath)
        : Mesh(),
        m_meshName{ meshName },
        m_rootDir{ rootDir },
        m_meshPath{ meshPath }
    {
    }

    ModelMesh::~ModelMesh()
    {
        KI_INFO(fmt::format("MODEL_MESH: delete - {}", str()));
        m_vertices.clear();
    }

    const std::string ModelMesh::str() const noexcept
    {
        return fmt::format(
            "<MODEL: id={}, rootDir={}, meshPath={}, name={}>",
            m_id, m_rootDir, m_meshPath, m_meshName);
    }

    const AABB ModelMesh::calculateAABB() const {
        glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

        for (auto&& vertex : m_vertices)
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

    const std::vector<Material>& ModelMesh::getMaterials() const
    {
        return m_materials;
    }

    kigl::GLVertexArray* ModelMesh::prepareRT(
        const Assets& assets,
        Registry* registry)
    {
        if (m_prepared) return m_vao;
        m_prepared = true;

        m_vertexVBO.prepare(*this);

        // NOTE KI no need for thexe any longer (they are in buffers now)
        // NOTE KI CANNOT clear vertices due to mesh sharing via ModelRegistry
        m_indexCount = static_cast<ki::uint>(m_indeces.size());
        m_indeces.clear();
        //m_vertices.clear();

        m_vao = registry->m_modelRegistry->registerModelVBO(m_vertexVBO);
        return m_vao;
    }

    void ModelMesh::prepareMaterials(
        MaterialVBO& materialVBO)
    {
        ModelMaterialInit init;
        init.prepare(*this, materialVBO);
    }

    void ModelMesh::prepareDrawOptions(
        backend::DrawOptions& drawOptions)
    {
        drawOptions.type = backend::DrawOptions::Type::elements;
        drawOptions.mode = drawOptions.tessellation ? GL_PATCHES : GL_TRIANGLES;
        drawOptions.indexCount = m_indexCount * 3;
        drawOptions.vertexOffset = static_cast<ki::uint>(m_vertexVBO.m_vertexOffset);
        drawOptions.indexOffset = static_cast<ki::uint>(m_vertexVBO.m_indexOffset);
    }
}