#include "QuadMesh.h"

#include "ki/GL.h"

#include "asset/Sphere.h"

const float VERTICES[] = {
    // pos              // normal         // tangent        //mat // tex
    -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
};

//const int INDECES[] = {
//    0, 1, 2,
//    2, 1, 3,
//};

//const int VERTEX_COUNT = 6;

namespace {
#pragma pack(push, 1)
    struct TexVBO {
        // TODO KI *BROKEN* if material is anything else than first
        // => completely broken with DSA
        // = *COULD* be related old "disappearing" materials issues?!?
        unsigned int material;
        glm::vec3 pos;
        KI_VEC10 normal;
        KI_VEC10 tangent;
        KI_UV16 texCoords;
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

bool QuadMesh::hasReflection() const
{
    return m_material.reflection;
}

bool QuadMesh::hasRefraction() const
{
    return m_material.refraction;
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

void QuadMesh::calculateVolume() {
    // NOTE KI calculate in 3D
    auto diam = std::sqrt(2 * 2 + 2 * 2 + 2 * 2);
    setVolume(std::make_unique<Sphere>(glm::vec3{ 0, 0, 0 }, diam / 2.f));
}

void QuadMesh::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    m_buffers.prepare(false);
    prepareMaterials(assets);
    prepareBuffers(m_buffers);
}

void QuadMesh::prepareMaterials(const Assets& assets)
{
    {
        Material& material = m_material;

        unsigned int texCount = 0;
        material.materialIndex = 0;

        material.prepare(assets);

        for (auto& tex : material.textures) {
            if (!tex.texture) continue;
            tex.texIndex = texCount++;
            m_textureIDs.push_back(tex.texture->textureID);
        }

        // NOTE KI second iteration to set unitIndex after texCount
        std::map<GLuint, bool> assignedUnits;
        std::map<GLuint, bool> assignedTextures;
        int unitIndex = 0;
        for (auto& tex : material.textures) {
            if (!tex.texture) continue;
            if (tex.texture->unitIndex < 0) {
                tex.texture->unitIndex = Texture::nextUnitIndex();
            }
            tex.unitIndex = tex.texture->unitIndex;

            if (assignedTextures[tex.texture->textureID]) continue;

            // NOTE KI conflict resolution if random conflict happens
            while (assignedUnits[tex.unitIndex] == true) {
                tex.unitIndex = unitIndex++;
            }
            assignedUnits[tex.unitIndex] = true;
            assignedTextures[tex.texture->textureID] = true;
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

void QuadMesh::prepareBuffers(MeshBuffers& curr)
{
    prepareVBO(curr);
}

void QuadMesh::prepareVBO(MeshBuffers& curr)
{
    const int vao = curr.VAO;

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(TexVBO);
    void* vboBuffer = new unsigned char[stride_size * 4];

    {
        constexpr int row_size = 12;// sizeof(VERTICES) / 4;

        TexVBO* vbo = (TexVBO*)vboBuffer;
        for (int i = 0; i < 4; i++) {
            int base = i * row_size;

            vbo->pos.x = VERTICES[base++];
            vbo->pos.y = VERTICES[base++];
            vbo->pos.z = VERTICES[base++];

            vbo->normal.x = (int)(VERTICES[base++] * SCALE_VEC10);
            vbo->normal.y = (int)(VERTICES[base++] * SCALE_VEC10);
            vbo->normal.z = (int)(VERTICES[base++] * SCALE_VEC10);
            vbo->normal.not_used = 0;

            vbo->tangent.x = (int)(VERTICES[base++] * SCALE_VEC10);
            vbo->tangent.y = (int)(VERTICES[base++] * SCALE_VEC10);
            vbo->tangent.z = (int)(VERTICES[base++] * SCALE_VEC10);
            vbo->tangent.not_used = 0;

            // NOTE KI hardcoded single material
            vbo->material = VERTICES[base++];

            vbo->texCoords.u = (int)(VERTICES[base++] * SCALE_UV16);
            vbo->texCoords.v = (int)(VERTICES[base++] * SCALE_UV16);

            vbo++;
        }
    }

    glNamedBufferStorage(curr.VBO, stride_size * 4, vboBuffer, 0);
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
        glVertexArrayAttribIFormat(vao, ATTR_MATERIAL_INDEX, 1, GL_UNSIGNED_INT, offsetof(TexVBO, material));

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
    Shader* shader,
    bool bindMaterials) noexcept
{
    if (bindMaterials) {
        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, m_materialsUboId, 0, m_materialsUboSize);

        m_material.bindArray(ctx, shader);
    }

    glBindVertexArray(m_buffers.VAO);
}

void QuadMesh::drawInstanced(const RenderContext& ctx, int instanceCount) noexcept
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);
    //glDrawElementsInstanced(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0, instanceCount);
}
