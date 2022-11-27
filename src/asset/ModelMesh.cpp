#include "ModelMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"

#include "scene/RenderContext.h"

namespace {
#pragma pack(push, 1)
    struct MaterialVBO {
        // NOTE KI uint DOES NOT work well in vertex attrs; data gets corrupted
        // => use float
        float material;
    };
#pragma pack(pop)
}

ModelMesh::ModelMesh(
    const std::string& name,
    const std::string& meshName)
    : ModelMesh(name, meshName, "")
{
}

ModelMesh::ModelMesh(
    const std::string& name,
    const std::string& meshName,
    const std::string& meshPath)
    : Mesh(name),
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
        "<MODEL: {} - mesh={}/{}, {}>",
        m_name, m_meshPath, m_meshName, m_buffers.str());
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

    m_buffers.prepare(false, true, true);
    prepareBuffers(m_buffers);
}

void ModelMesh::prepareBuffers(MeshBuffers& curr)
{
    KI_DEBUG_SB(fmt::format("{} - curr={}", str(), curr.str()));

    prepareVertexVBO(curr);
    prepareMaterialVBO(curr);
    prepareEBO(curr);

    // NOTE KI no need for thexe any longer (they are in buffers now)
    m_triCount = m_tris.size();
    m_vertices.clear();
    m_tris.clear();
}

void ModelMesh::prepareVertexVBO(MeshBuffers& curr)
{
    m_vertex.prepare(*this);
    m_vertex.prepareVAO(curr.VAO);
}

void ModelMesh::prepareMaterialVBO(MeshBuffers& curr)
{
    const int vao = curr.VAO;

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(MaterialVBO);
    const int sz = stride_size * m_vertices.size();

    MaterialVBO* buffer = (MaterialVBO*)new unsigned char[sz];
    memset(buffer, 0, sz);

    {
        MaterialVBO* vbo = buffer;
        for (int i = 0; i < m_vertices.size(); i++) {
            const auto& vertex = m_vertices[i];
            const auto& m = Material::findID(vertex.materialID, m_materials);

            // TODO KI should use noticeable value for missing
            // => would trigger undefined array access in render side
            vbo->material = m ? (m->m_registeredIndex) : 0;

            vbo++;
        }
    }

    assert(buffer->material >= 0 && buffer->material < MAX_MATERIAL_COUNT);
    glNamedBufferStorage(curr.VBO_MATERIAL, sz, buffer, 0);
    delete[] buffer;

    glVertexArrayVertexBuffer(vao, VBO_MATERIAL_BINDING, curr.VBO_MATERIAL, 0, stride_size);
    {
        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);

        // materialID attr
        glVertexArrayAttribFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(MaterialVBO, material));

        glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_MATERIAL_BINDING);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_MATERIAL_BINDING, 0);
    }
}

void ModelMesh::prepareEBO(MeshBuffers& curr)
{
    const int vao = curr.VAO;

    // EBO == IBO ?!?
    const int index_count = m_tris.size() * 3;
    unsigned int* vertexEboBuffer = new unsigned int[index_count];

    for (int i = 0; i < m_tris.size(); i++) {
        const auto& vi = m_tris[i];
        const int base = i * 3;
        vertexEboBuffer[base + 0] = vi[0];
        vertexEboBuffer[base + 1] = vi[1];
        vertexEboBuffer[base + 2] = vi[2];
    }

    glNamedBufferStorage(curr.EBO, sizeof(unsigned int) * index_count, vertexEboBuffer, 0);
    delete[] vertexEboBuffer;

    glVertexArrayElementBuffer(vao, curr.EBO);
}

void ModelMesh::bind(
    const RenderContext& ctx,
    Shader* shader) noexcept
{
    glBindVertexArray(m_buffers.VAO);
}

void ModelMesh::drawInstanced(const RenderContext& ctx, int instanceCount) noexcept
{
    KI_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, m_triCount * 3, GL_UNSIGNED_INT, 0, instanceCount));
}
