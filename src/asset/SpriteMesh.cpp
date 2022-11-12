#include "SpriteMesh.h"

#include "ki/GL.h"

#include "asset/Sphere.h"

namespace {
#pragma pack(push, 1)
    struct TexVBO {
        // TODO KI *BROKEN* if material is anything else than first
        // => completely broken with DSA
        // = *COULD* be related old "disappearing" materials issues?!?
        unsigned int material;
    };
#pragma pack(pop)
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

bool SpriteMesh::hasReflection() const
{
    return m_material.reflection;
}

bool SpriteMesh::hasRefraction() const
{
    return m_material.refraction;
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

void SpriteMesh::calculateVolume() {
    // NOTE KI calculate in 3D
    auto diam = std::sqrt(2 * 2 + 2 * 2 + 2 * 2);
    setVolume(std::make_unique<Sphere>(glm::vec3{ 0, 0, 0 }, diam / 2.f));
}

void SpriteMesh::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    m_buffers.prepare(false);
    prepareMaterials(assets);
    prepareBuffers(m_buffers);
}

void SpriteMesh::prepareMaterials(const Assets& assets)
{
    {
        Material& material = m_material;

        material.m_index = 0;

        material.prepare(assets);

        for (auto& tex : material.m_textures) {
            if (!tex.texture) continue;
            tex.m_texIndex = tex.texture->m_texIndex;
            m_textureIDs.push_back(tex.texture->m_textureID);
        }
    }

    // materials
    {
        int sz_single = sizeof(MaterialUBO);
        m_materialsUboSize = sz_single * 1;

        MaterialsUBOSingle materialsUbo{};

        materialsUbo.materials[0] = m_material.toUBO();

        glCreateBuffers(1, &m_materialsUboId);
        glNamedBufferStorage(m_materialsUboId, m_materialsUboSize, &materialsUbo, 0);
    }
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
    constexpr int stride_size = sizeof(TexVBO);
    TexVBO vbo;
    vbo.material = 0;

    glNamedBufferStorage(curr.VBO, stride_size, &vbo, 0);

    glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, curr.VBO, 0, stride_size);
    {
        glEnableVertexArrayAttrib(vao, ATTR_MATERIAL_INDEX);

        glVertexArrayAttribIFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_UNSIGNED_INT, offsetof(TexVBO, material));

        glVertexArrayAttribBinding(vao, ATTR_MATERIAL_INDEX, VBO_VERTEX_BINDING);
    }
}

void SpriteMesh::bind(
    const RenderContext& ctx,
    Shader* shader,
    bool bindMaterials) noexcept
{
    if (bindMaterials) {
        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, m_materialsUboId, 0, m_materialsUboSize);

        m_material.bindArray(ctx, shader);
    }

    glBindVertexArray(m_buffers.VAO);
}

void SpriteMesh::drawInstanced(const RenderContext& ctx, int instanceCount) noexcept
{
    glDrawArraysInstanced(GL_POINTS, 0, 1, instanceCount);
}
