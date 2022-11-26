#include "QuadMesh.h"

#include "ki/GL.h"

#include "asset/Sphere.h"

#include "scene/RenderContext.h"

namespace {
    const float VERTICES[] = {
        // pos              // normal         // tangent        //mat // tex
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    constexpr int ROW_SIZE = 12;
    constexpr int VERTEX_COUNT = 4;

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


QuadMesh::QuadMesh(const std::string& name)
    : Mesh(name)
{
}

QuadMesh::~QuadMesh()
{
}

const std::string QuadMesh::str() const
{
    return "<QUAD_MESH: " + m_name + ">";
}

Material* QuadMesh::findMaterial(std::function<bool(const Material&)> fn)
{
    if (fn(m_material)) return &m_material;
    return nullptr;
}

void QuadMesh::modifyMaterials(std::function<void(Material&)> fn)
{
    fn(m_material);
}

void QuadMesh::prepareVolume() {
    const auto& aabb = calculateAABB();
    setAABB(aabb);

    setVolume(std::make_unique<Sphere>(
        (aabb.m_max + aabb.m_min) * 0.5f,
        // NOTE KI *radius* not diam needed
        glm::length(aabb.m_min - aabb.m_max) * 0.5f));
}

const AABB& QuadMesh::calculateAABB() const
{
    return { glm::vec3{-1.f, -1.f, 0.f}, glm::vec3{1.f, 1.f, 0.f}, true };
}

void QuadMesh::prepare(
    const Assets& assets,
    NodeRegistry& registry)
{
    if (m_prepared) return;
    m_prepared = true;

    m_buffers.prepare(false);
    prepareBuffers(m_buffers);
}

void QuadMesh::prepareBuffers(MeshBuffers& curr)
{
    prepareVBO(curr);
}

void QuadMesh::prepareVBO(MeshBuffers& curr)
{
    const int vao = curr.VAO;

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(TexVBO);
    const int sz = stride_size * VERTEX_COUNT;

    TexVBO* vboBuffer = (TexVBO*)new unsigned char[sz];
    memset(vboBuffer, 0, sz);

    {
        constexpr int row_size = ROW_SIZE;

        TexVBO* vbo = vboBuffer;
        for (int i = 0; i < VERTEX_COUNT; i++) {
            int base = i * row_size;

            vbo->pos.x = VERTICES[base++];
            vbo->pos.y = VERTICES[base++];
            vbo->pos.z = VERTICES[base++];

            vbo->normal.x = (int)(VERTICES[base++] * ki::SCALE_VEC10);
            vbo->normal.y = (int)(VERTICES[base++] * ki::SCALE_VEC10);
            vbo->normal.z = (int)(VERTICES[base++] * ki::SCALE_VEC10);

            vbo->tangent.x = (int)(VERTICES[base++] * ki::SCALE_VEC10);
            vbo->tangent.y = (int)(VERTICES[base++] * ki::SCALE_VEC10);
            vbo->tangent.z = (int)(VERTICES[base++] * ki::SCALE_VEC10);

            // NOTE KI hardcoded single material
            base++;
            vbo->material = m_material.m_registeredIndex;

            vbo->texCoords.u = (int)(VERTICES[base++] * ki::SCALE_UV16);
            vbo->texCoords.v = (int)(VERTICES[base++] * ki::SCALE_UV16);

            assert(vbo->material >= 0 && vbo->material < MAX_MATERIAL_COUNT);

            vbo++;
        }
    }

    glNamedBufferStorage(curr.VBO, sz, vboBuffer, 0);
    delete[] vboBuffer;

    glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, curr.VBO, 0, stride_size);
    {
        glEnableVertexArrayAttrib(vao, ATTR_POS);
        glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
        glEnableVertexArrayAttrib(vao, ATTR_TANGENT);
        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);
        glEnableVertexArrayAttrib(vao, ATTR_TEX);

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
    }
}

void QuadMesh::bind(
    const RenderContext& ctx,
    Shader* shader) noexcept
{
    glBindVertexArray(m_buffers.VAO);
}

void QuadMesh::drawInstanced(const RenderContext& ctx, int instanceCount) noexcept
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);
    //glDrawElementsInstanced(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0, instanceCount);
}
