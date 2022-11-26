#include "SpriteMesh.h"

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

    const AABB SPRITE_AABB = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };
}


SpriteMesh::SpriteMesh(const std::string& name)
    : Mesh(name)
{
}

SpriteMesh::~SpriteMesh()
{
}

const std::string SpriteMesh::str() const
{
    return "<SPRITE_MESH: " + m_name + ">";
}

Material* SpriteMesh::findMaterial(std::function<bool(const Material&)> fn)
{
    if (fn(m_material)) return &m_material;
    return nullptr;
}

void SpriteMesh::modifyMaterials(std::function<void(Material&)> fn)
{
    fn(m_material);
}

void SpriteMesh::prepareVolume() {
    const auto& aabb = calculateAABB();
    setAABB(aabb);
    setVolume(std::make_unique<Sphere>(
        (aabb.m_max + aabb.m_min) * 0.5f,
        // NOTE KI *radius* not diam needed
        glm::length(aabb.m_min - aabb.m_max) * 0.5f));
}

const AABB& SpriteMesh::calculateAABB() const
{
    return SPRITE_AABB;
}

void SpriteMesh::prepare(
    const Assets& assets,
    NodeRegistry& registry)
{
    if (m_prepared) return;
    m_prepared = true;

    m_buffers.prepare(false, true, false);
    prepareBuffers(m_buffers);
}

void SpriteMesh::prepareBuffers(MeshBuffers& curr)
{
    prepareVBO(curr);
}

void SpriteMesh::prepareVBO(MeshBuffers& curr)
{
    //return;
    const int vao = curr.VAO;

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(MaterialVBO);
    MaterialVBO vbo;
    vbo.material = m_material.m_registeredIndex;

    glNamedBufferStorage(curr.VBO_MATERIAL, stride_size, &vbo, 0);

    glVertexArrayVertexBuffer(vao, VBO_MATERIAL_BINDING, curr.VBO_MATERIAL, 0, stride_size);
    {
        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);

        glVertexArrayAttribFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(MaterialVBO, material));

        glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_MATERIAL_BINDING);
    }
}

void SpriteMesh::bind(
    const RenderContext& ctx,
    Shader* shader) noexcept
{
    glBindVertexArray(m_buffers.VAO);
}

void SpriteMesh::drawInstanced(const RenderContext& ctx, int instanceCount) noexcept
{
    glDrawArraysInstanced(GL_POINTS, 0, 1, instanceCount);
}
