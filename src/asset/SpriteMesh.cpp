#include "SpriteMesh.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"

#include "scene/RenderContext.h"


namespace {
#pragma pack(push, 1)
    struct MaterialEntry {
        // NOTE KI uint DOES NOT work well in vertex attrs; data gets corrupted
        // => use float
        float material;
    };
#pragma pack(pop)

    const AABB SPRITE_AABB = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };
}


SpriteMesh::SpriteMesh()
    : Mesh()
{
}

SpriteMesh::~SpriteMesh()
{
}

const std::string SpriteMesh::str() const
{
    return fmt::format("<SPRITE: {}>", m_objectID);
}

const AABB& SpriteMesh::calculateAABB() const
{
    return SPRITE_AABB;
}

const std::vector<Material>& SpriteMesh::getMaterials() const
{
    return { m_material };
}

void SpriteMesh::prepare(
    const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;
}

void SpriteMesh::prepareMaterials(
    MaterialVBO& materialVBO)
{
    prepareMaterialVBO(materialVBO);
}

void SpriteMesh::prepareVAO(
    GLVertexArray& vao,
    MaterialVBO& materialVBO)
{
    glVertexArrayVertexBuffer(vao, VBO_MATERIAL_BINDING, materialVBO.m_vbo, 0, sizeof(MaterialEntry));
    {
        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);

        glVertexArrayAttribFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(MaterialEntry, material));

        glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_MATERIAL_BINDING);
    }
}

void SpriteMesh::prepareMaterialVBO(
    MaterialVBO& materialVBO)
{
    const auto& material = materialVBO.getMaterials()[0];

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(MaterialEntry);
    MaterialEntry vbo;
    vbo.material = material.m_registeredIndex;

    glNamedBufferStorage(materialVBO.m_vbo, stride_size, &vbo, 0);
}

void SpriteMesh::drawInstanced(const RenderContext& ctx, int instanceCount) const
{
    glDrawArraysInstanced(GL_POINTS, 0, 1, instanceCount);
}
