#include "QuadMesh.h"

#include "ki/GL.h"

#include "asset/Sphere.h"

#include "scene/RenderContext.h"

namespace {
    constexpr int VERTEX_COUNT = 4;

#pragma pack(push, 1)
    struct MaterialVBO {
        // NOTE KI uint DOES NOT work well in vertex attrs; data gets corrupted
        // => use float
        float material;
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

    m_buffers.prepare(false, true, false);

    registry.m_quad.prepareMesh(m_buffers.VAO);

    prepareBuffers(m_buffers);
}

void QuadMesh::prepareBuffers(MeshBuffers& curr)
{
    prepareMaterialVBO(curr);
}

void QuadMesh::prepareMaterialVBO(MeshBuffers& curr)
{
    const int vao = curr.VAO;

    // MaterialVBO
    {
        // https://paroj.github.io/gltut/Basic%20Optimization.html
        constexpr int stride_size = sizeof(MaterialVBO);
        {
            MaterialVBO buffer;
            buffer.material = m_material.m_registeredIndex;

            glNamedBufferStorage(curr.VBO_MATERIAL, stride_size, &buffer, 0);
        }

        glVertexArrayVertexBuffer(vao, VBO_MATERIAL_BINDING, curr.VBO_MATERIAL, 0, stride_size);
        {
            glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);

            // materialID attr
            glVertexArrayAttribFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_FLOAT, GL_FALSE, offsetof(MaterialVBO, material));

            glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_MATERIAL_BINDING);

            // TODO KI not 100% sure *HOW*
            // => ROW_SIZE * VERTEX_COUNT == 44
            // => Same material for all verteces
            glVertexArrayBindingDivisor(vao, VBO_MATERIAL_BINDING, 44);
        }
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
