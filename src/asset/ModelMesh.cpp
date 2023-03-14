#include "ModelMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"

#include "asset/ModelMaterialInit.h"

#include "registry/ModelRegistry.h"


namespace {
}

ModelMesh::ModelMesh(
    const std::string& meshName)
    : ModelMesh(meshName, "")
{
}

ModelMesh::ModelMesh(
    const std::string& meshName,
    const std::string& meshPath)
    : Mesh(),
    m_meshName(meshName),
    m_meshPath(meshPath)
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
        "<MODEL: {}, mesh={}/{}>",
        m_objectID, m_meshPath, m_meshName);
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

GLVertexArray* ModelMesh::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return m_vao;
    m_prepared = true;

    m_vertexVBO.prepare(*this);

    // NOTE KI no need for thexe any longer (they are in buffers now)
    // NOTE KI CANNOT clear vertices due to mesh sharing via ModelRegistry
    m_triCount = m_tris.size();
    m_tris.clear();
    //m_vertices.clear();

    m_vao = registry->m_modelRegistry->registerMeshVBO(m_vertexVBO);
    return m_vao;
}

void ModelMesh::prepareMaterials(
    MaterialVBO& materialVBO)
{
    ModelMaterialInit init;
    init.prepare(*this, materialVBO);
}

void ModelMesh::prepareVAO(
    GLVertexArray& vao,
    backend::DrawOptions& drawOptions)
{
    //m_vertexVBO.prepareVAO(vao);

    drawOptions.type = backend::DrawOptions::Type::elements;
    drawOptions.mode = drawOptions.tessellation ? GL_PATCHES : GL_TRIANGLES;
    drawOptions.indexCount = m_triCount * 3;
    drawOptions.vertexOffset = m_vertexVBO.m_vertexOffset;
    drawOptions.indexOffset = m_vertexVBO.m_indexOffset;
}
