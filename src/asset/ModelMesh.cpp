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

Material* ModelMesh::findMaterial(std::function<bool(const Material&)> fn)
{
    for (auto& material : m_materials) {
        if (fn(material)) return &material;
    }
    return nullptr;
}

void ModelMesh::modifyMaterials(std::function<void(Material&)> fn)
{
    for (auto& material : m_materials) {
        fn(material);
    }
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

void ModelMesh::prepare(
    const Assets& assets,
    NodeRegistry& registry)
{
    if (m_prepared) return;
    m_prepared = true;

    m_buffers.prepare(false, false, false);
    prepareBuffers(m_buffers);
}

void ModelMesh::prepareBuffers(MeshBuffers& curr)
{
    KI_DEBUG_SB(fmt::format("{} - curr={}", str(), curr.str()));

    m_vertexVBO.prepare(*this);
    m_vertexVBO.prepareVAO(curr.VAO);

    m_materialVBO.prepare(*this);
    m_materialVBO.prepareVAO(curr.VAO);

    // NOTE KI no need for thexe any longer (they are in buffers now)
    m_triCount = m_tris.size();
    m_vertices.clear();
    m_tris.clear();
}

void ModelMesh::bind(
    const RenderContext& ctx,
    Shader* shader) noexcept
{
    glBindVertexArray(m_buffers.VAO);
}

void ModelMesh::drawInstanced(const RenderContext& ctx, int instanceCount) const
{
    KI_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, m_triCount * 3, GL_UNSIGNED_INT, 0, instanceCount));
}
