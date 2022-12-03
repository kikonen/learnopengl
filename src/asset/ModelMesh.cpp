#include "ModelMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"

#include "scene/RenderContext.h"

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
    KI_INFO_SB("MODEL_MESH: delete " << str());
    m_vertices.clear();
}

const std::string ModelMesh::str() const
{
    return fmt::format(
        "<MODEL: {}, mesh={}/{}>",
        m_objectID, m_meshPath, m_meshName);
}

void ModelMesh::prepareVolume() {
    const auto& aabb = calculateAABB();
    setAABB(aabb);

    setVolume(std::make_unique<Sphere>(
        (aabb.m_max + aabb.m_min) * 0.5f,
        // NOTE KI *radius* not diam needed
        glm::length(aabb.m_min - aabb.m_max) * 0.5f));
}

const AABB& ModelMesh::calculateAABB() const {
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

void ModelMesh::prepare(
    const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    m_vertexVBO.prepare(*this);

    // NOTE KI no need for thexe any longer (they are in buffers now)
    // NOTE KI CANNOT clear vertices due to mesh sharing via MeshRegistry
    m_triCount = m_tris.size();
    m_tris.clear();
    //m_vertices.clear();
}

void ModelMesh::prepareMaterials(
    MaterialVBO& materialVBO)
{
    materialVBO.prepare(*this);
}

void ModelMesh::prepareVAO(
    GLVertexArray& vao,
    MaterialVBO& materialVBO)
{
    m_vertexVBO.prepareVAO(vao);
    materialVBO.prepareVAO(vao);
}

void ModelMesh::drawInstanced(const RenderContext& ctx, int instanceCount) const
{
    KI_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, m_triCount * 3, GL_UNSIGNED_INT, (void*)m_vertexVBO.m_index_offset, instanceCount));
}
