#include "ModelMesh.h"

#include <glm/glm.hpp>

#include <string>
#include <algorithm>

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"

namespace {
#pragma pack(push, 1)
    struct TexVBO {
        glm::vec3 pos;
        ki::VEC10 normal;
        ki::VEC10 tangent;
        // NOTE KI uint DOES NOT work well in vertex attrs; data gets corrupted
        // => use float
        float material;
        ki::UV16 texCoords;
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
        "<MODEL: {} - mesh={}/{}, vao={}, vbo={}, ebo={}>",
        m_name, m_meshPath, m_meshName, m_buffers.VAO, m_buffers.VBO, m_buffers.EBO);
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

void ModelMesh::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    m_buffers.prepare(true);
    prepareMaterials(assets);
    prepareBuffers(m_buffers);
}

void ModelMesh::prepareMaterials(const Assets& assets)
{
    m_reflection = false;
    m_refraction = false;

    {
        int materialIndex = 0;

        for (auto& material : m_materials) {
            material.m_index = materialIndex++;
            assert(material.m_index < MATERIAL_COUNT);

            m_reflection |= material.reflection > 0;
            m_refraction |= material.refraction > 0;

            material.prepare(assets);

            for (auto& tex : material.m_textures) {
                if (!tex.texture) continue;
                tex.m_texIndex = tex.texture->m_texIndex;
                m_textureIDs.push_back(tex.texture->m_textureID);
            }
        }
    }

    // materials
    {
        int sz_single = sizeof(MaterialUBO);
        //int sz = sizeof(MaterialUBO);
        m_materialsUboSize = sz_single * m_materials.size();

        MaterialsUBO materialsUbo{};

        for (auto& material : m_materials) {
            materialsUbo.materials[material.m_index] = material.toUBO();
        }

        glCreateBuffers(1, &m_materialsUboId);
        glNamedBufferStorage(m_materialsUboId, m_materialsUboSize, &materialsUbo, 0);
    }
}

void ModelMesh::prepareBuffers(MeshBuffers& curr)
{
    KI_DEBUG_SB(str() + " - VAO = " << curr.VAO << ", VBO = " << curr.VBO << ", EBO = " << curr.EBO);

    prepareVBO(curr);
    prepareEBO(curr);

    // NOTE KI no need for thexe any longer (they are in buffers now)
    m_triCount = m_tris.size();
    m_vertices.clear();
    m_tris.clear();
}

void ModelMesh::prepareVBO(MeshBuffers& curr)
{
    const int vao = curr.VAO;

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(TexVBO);
    const int sz = stride_size * m_vertices.size();

    TexVBO* vboBuffer = (TexVBO*)new unsigned char[sz];
    memset(vboBuffer, 0, sz);

    {
        TexVBO* vbo = vboBuffer;
        for (int i = 0; i < m_vertices.size(); i++) {
            const Vertex& vertex = m_vertices[i];
            const glm::vec3& p = vertex.pos;
            const glm::vec3& n = vertex.normal;
            const glm::vec3& tan = vertex.tangent;
            const Material* m = Material::findID(vertex.materialID, m_materials);
            const glm::vec2& t = vertex.texture;

            vbo->pos.x = p.x;
            vbo->pos.y = p.y;
            vbo->pos.z = p.z;

            vbo->normal.x = (int)(n.x * ki::SCALE_VEC10);
            vbo->normal.y = (int)(n.y * ki::SCALE_VEC10);
            vbo->normal.z = (int)(n.z * ki::SCALE_VEC10);

            vbo->tangent.x = (int)(tan.x * ki::SCALE_VEC10);
            vbo->tangent.y = (int)(tan.y * ki::SCALE_VEC10);
            vbo->tangent.z = (int)(tan.z * ki::SCALE_VEC10);

            //vbo->normal.x = n.x;
            //vbo->normal.y = n.y;
            //vbo->normal.z = n.z;

            //vbo->tangent.x = tan.x;
            //vbo->tangent.y = tan.y;
            //vbo->tangent.z = tan.z;

            // TODO KI should use noticeable value for missing
            // => would trigger undefined array access in render side
            vbo->material = m ? m->m_index : 0;

            vbo->texCoords.u = (int)(t.x * ki::SCALE_UV16);
            vbo->texCoords.v = (int)(t.y * ki::SCALE_UV16);
            //vbo->texCoords.x = t.x;
            //vbo->texCoords.y = t.y;

            assert(vbo->material >= 0 && vbo->material < MAX_MATERIAL_COUNT);

            vbo++;
        }
    }

    assert(vboBuffer->material >= 0 && vboBuffer->material < MAX_MATERIAL_COUNT);
    glNamedBufferStorage(curr.VBO, sz, vboBuffer, 0);
    delete[] vboBuffer;

    glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, curr.VBO, 0, stride_size);
    {
        glEnableVertexArrayAttrib(vao, ATTR_POS);
        glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
        glEnableVertexArrayAttrib(vao, ATTR_TANGENT);
        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);
        glEnableVertexArrayAttrib(vao, ATTR_TEX);

        // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        //
        // vertex attr
        glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(TexVBO, pos));

        // normal attr
        glVertexArrayAttribFormat(vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(TexVBO, normal));

        // tangent attr
        glVertexArrayAttribFormat(vao, ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(TexVBO, tangent));

        // materialID attr
        glVertexArrayAttribFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(TexVBO, material));

        // texture attr
        glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(TexVBO, texCoords));

        glVertexArrayAttribBinding(vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_NORMAL, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TANGENT, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TEX, VBO_VERTEX_BINDING);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_VERTEX_BINDING, 0);
    }
}

void ModelMesh::prepareEBO(MeshBuffers& curr)
{
    const int vao = curr.VAO;

    // EBO == IBO ?!?
    const int index_count = m_tris.size() * 3;
    unsigned int* vertexEboBuffer = new unsigned int[index_count];

    for (int i = 0; i < m_tris.size(); i++) {
        const glm::uvec3& vi = m_tris[i];
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
    Shader* shader,
    bool bindMaterials) noexcept
{
    if (bindMaterials) {
        //glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, m_materialsUboId, 0, m_materialsUboSize);
         glBindBufferBase(GL_UNIFORM_BUFFER, UBO_MATERIALS, m_materialsUboId);


        for (auto& material : m_materials) {
            material.bindArray(ctx, shader);
        }
    }

    glBindVertexArray(m_buffers.VAO);
}

void ModelMesh::drawInstanced(const RenderContext& ctx, int instanceCount) noexcept
{
    KI_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, m_triCount * 3, GL_UNSIGNED_INT, 0, instanceCount));
}
