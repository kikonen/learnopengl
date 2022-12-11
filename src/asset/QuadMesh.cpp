#include "QuadMesh.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Sphere.h"

#include "registry/MaterialEntry.h"

#include "scene/RenderContext.h"

namespace {
    constexpr int VERTEX_COUNT = 4;

    const AABB QUAD_AABB = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };

    QuadVBO m_quad;
}


QuadMesh::QuadMesh()
    : Mesh()
{
}

QuadMesh::~QuadMesh()
{
}

const std::string QuadMesh::str() const
{
    return fmt::format("<QUAD: {}>", m_objectID);
}

const AABB& QuadMesh::calculateAABB() const
{
    return QUAD_AABB;
}

const std::vector<Material>& QuadMesh::getMaterials() const
{
    return { m_material };
}

void QuadMesh::prepare(
    const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    m_quad.prepare();
}

void QuadMesh::prepareMaterials(
    MaterialVBO& materialVBO)
{
    prepareMaterialVBO(materialVBO);
}

void QuadMesh::prepareVAO(
    GLVertexArray& vao,
    MaterialVBO& materialVBO)
{
    m_quad.prepareVAO(vao);

    glVertexArrayVertexBuffer(vao, VBO_MATERIAL_BINDING, materialVBO.m_vbo, 0, sizeof(MaterialEntry));
    {
        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);

        // materialID attr
        glVertexArrayAttribFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(MaterialEntry, material));

        glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_MATERIAL_BINDING);
    }
}

void QuadMesh::prepareMaterialVBO(
    MaterialVBO& materialVBO)
{
    const auto& material = materialVBO.getMaterials()[0];

    // MaterialVBO
    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(MaterialEntry);
    // NOTE KI single DOES NOT work due to logic how intanced rendering
    // and glVertexArrayBindingDivisor work (MUST seemingly match instanced count)
    {
        const int sz = stride_size * VERTEX_COUNT;

        MaterialEntry* buffer = (MaterialEntry*)new unsigned char[sz];
        memset(buffer, 0, sz);

        // NOTE KI hardcoded single material
        constexpr int row_size = 1;

        MaterialEntry* vbo = buffer;
        for (int i = 0; i < VERTEX_COUNT; i++) {
            int base = i * row_size;

            // NOTE KI hardcoded single material
            base++;
            vbo->material = material.m_registeredIndex;

            assert(vbo->material >= 0 && vbo->material < MAX_MATERIAL_COUNT);

            vbo++;
        }

        glNamedBufferStorage(materialVBO.m_vbo, sz, buffer, 0);
        delete[] buffer;
    }
}

void QuadMesh::drawInstanced(const RenderContext& ctx, int instanceCount) const
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);
}
