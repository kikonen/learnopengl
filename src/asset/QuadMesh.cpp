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
        glm::vec3 pos;
        KI_VEC10 normal;
        KI_VEC10 tangent;
        unsigned char material;
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
    prepareBuffers(m_buffers);

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
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(curr.VAO);

    // VBO
    {
        int row_size = 12;// sizeof(VERTICES) / 4;

        // https://paroj.github.io/gltut/Basic%20Optimization.html
        const int stride_size = sizeof(TexVBO);
        void* vboBuffer = new unsigned char[stride_size * 4];

        {
            TexVBO* vbo = (TexVBO*)vboBuffer;
            for (int i = 0; i < 4; i++) {
                int base = i * row_size;

                vbo->pos.x = VERTICES[base++];
                vbo->pos.y = VERTICES[base++];
                vbo->pos.z = VERTICES[base++];

                vbo->normal.x = (int)(VERTICES[base++] * SCALE_VEC10);
                vbo->normal.y = (int)(VERTICES[base++] * SCALE_VEC10);
                vbo->normal.z = (int)(VERTICES[base++] * SCALE_VEC10);

                vbo->tangent.x = (int)(VERTICES[base++] * SCALE_VEC10);
                vbo->tangent.y = (int)(VERTICES[base++] * SCALE_VEC10);
                vbo->tangent.z = (int)(VERTICES[base++] * SCALE_VEC10);

                vbo->material = VERTICES[base++];

                vbo->texCoords.u = (int)(VERTICES[base++] * SCALE_UV16);
                vbo->texCoords.v = (int)(VERTICES[base++] * SCALE_UV16);

                vbo++;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, curr.VBO);
        KI_GL_CALL(glBufferData(GL_ARRAY_BUFFER, stride_size * 4, vboBuffer, GL_STATIC_DRAW));
        delete[] vboBuffer;

        int offset = 0;

        // vertex attr
        KI_GL_CALL(glVertexAttribPointer(ATTR_POS, 3, GL_FLOAT, GL_FALSE, stride_size, (void*)offset));
        offset += sizeof(glm::vec3);

        // normal attr
        KI_GL_CALL(glVertexAttribPointer(ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, stride_size, (void*)offset));
        offset += sizeof(KI_VEC10);

        // tangent attr
        KI_GL_CALL(glVertexAttribPointer(ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, stride_size, (void*)offset));
        offset += sizeof(KI_VEC10);

        // materialID attr
        KI_GL_CALL(glVertexAttribIPointer(ATTR_MATERIAL_INDEX, 1, GL_UNSIGNED_BYTE, stride_size, (void*)offset));
        offset += sizeof(unsigned char);

        // texture attr
        KI_GL_CALL(glVertexAttribPointer(ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride_size, (void*)offset));

        glEnableVertexAttribArray(ATTR_POS);
        glEnableVertexAttribArray(ATTR_NORMAL);
        glEnableVertexAttribArray(ATTR_TANGENT);
        glEnableVertexAttribArray(ATTR_MATERIAL_INDEX);
        glEnableVertexAttribArray(ATTR_TEX);
    }

    // EBO
    //{
    //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr.EBO);
    //    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDECES), &INDECES, GL_STATIC_DRAW);
    //}

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void QuadMesh::bind(
    const RenderContext& ctx,
    Shader* shader,
    bool bindMaterials)
{
    if (bindMaterials) {
        glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, m_materialsUboId, 0, m_materialsUboSize);
    }

    glBindVertexArray(m_buffers.VAO);

    if (bindMaterials) {
        m_material.bindArray(ctx, shader, 0, true);
    }

    //if (!m_textureIDs.empty()) {
    //    ctx.state.bindTextures(m_unitIndexFirst, m_textureIDs);
    //}
}

void QuadMesh::drawInstanced(const RenderContext& ctx, int instanceCount)
{
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, instanceCount);
    //glDrawElementsInstanced(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0, instanceCount);
}
