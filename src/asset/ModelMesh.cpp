#include "ModelMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fmt/format.h>

#include "ki/GL.h"

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

bool ModelMesh::hasReflection() const
{
    return m_reflection;
}

bool ModelMesh::hasRefraction() const
{
    return m_refraction;
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

void ModelMesh::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    m_buffers.prepare(true);

    m_reflection = false;
    m_refraction = false;

    {
        int materialIndex = 0;
        unsigned int unitIndex = 0;

        for (auto& material : m_materials) {
            material.materialIndex = materialIndex++;
            assert(material.materialIndex < MATERIAL_COUNT);

            m_reflection |= material.reflection > 0;
            m_refraction |= material.refraction > 0;

            material.prepare(assets);

            for (auto& tex : material.textures) {
                if (!tex.texture) continue;
                tex.unitIndex = unitIndex++;
                m_textureIDs.push_back(tex.texture->textureID);
            }
        }
    }

    prepareBuffers(m_buffers);

    // materials
    {
        int sz_single = sizeof(MaterialUBO);
        int sz = sizeof(MaterialsUBO);
        m_materialsUboSize = sz;

        MaterialsUBO materialsUbo{};

        for (auto& material : m_materials) {
            materialsUbo.materials[material.materialIndex] = material.toUBO();
        }

        glCreateBuffers(1, &m_materialsUboId);
        glNamedBufferStorage(m_materialsUboId, sz, &materialsUbo, 0);
    }
}

void ModelMesh::prepareBuffers(MeshBuffers& curr)
{
    KI_DEBUG_SB(str() + " - VAO = " << curr.VAO << ", VBO = " << curr.VBO << ", EBO = " << curr.EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(curr.VAO);

    // VBO
    {
        // https://paroj.github.io/gltut/Basic%20Optimization.html
        const int stride_size = sizeof(TexVBO);
        void* vboBuffer = new unsigned char[stride_size * m_vertices.size()];

        {
            TexVBO* vbo = (TexVBO*)vboBuffer;
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

                vbo->normal.x = (int)(n.x * SCALE_VEC10);
                vbo->normal.y = (int)(n.y * SCALE_VEC10);
                vbo->normal.z = (int)(n.z * SCALE_VEC10);

                vbo->tangent.x = (int)(tan.x * SCALE_VEC10);
                vbo->tangent.y = (int)(tan.y * SCALE_VEC10);
                vbo->tangent.z = (int)(tan.z * SCALE_VEC10);

                // TODO KI should use noticeable value for missing
                vbo->material = m ? m->materialIndex : 0;

                vbo->texCoords.u = (int)(t.x * SCALE_UV16);
                vbo->texCoords.v = (int)(t.y * SCALE_UV16);

                vbo++;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, curr.VBO);
        KI_GL_CALL(glBufferData(GL_ARRAY_BUFFER, stride_size * m_vertices.size(), vboBuffer, GL_STATIC_DRAW));

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
    {
        int index_count = m_tris.size() * 3;
        unsigned int* vertexEboBuffer = new unsigned int[index_count];

        for (int i = 0; i < m_tris.size(); i++) {
            const glm::uvec3& vi = m_tris[i];
            const int base = i * 3;
            vertexEboBuffer[base + 0] = vi[0];
            vertexEboBuffer[base + 1] = vi[1];
            vertexEboBuffer[base + 2] = vi[2];
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * index_count, vertexEboBuffer, GL_STATIC_DRAW);

        delete[] vertexEboBuffer;
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // NOTE KI no need for thexe any longer (they are in buffers now)
    m_triCount = m_tris.size();
    m_vertices.clear();
    m_tris.clear();
}

void ModelMesh::bind(const RenderContext& ctx, Shader* shader)
{
    glBindBufferRange(GL_UNIFORM_BUFFER, UBO_MATERIALS, m_materialsUboId, 0, m_materialsUboSize);

    glBindVertexArray(m_buffers.VAO);

    for (auto& material : m_materials) {
        material.bindArray(shader, material.materialIndex, false);
    }

    if (!m_textureIDs.empty()) {
        glBindTextures(m_unitIndexFirst, m_textureIDs.size(), &m_textureIDs[0]);
    }
}

void ModelMesh::drawInstanced(const RenderContext& ctx, int instanceCount)
{
    KI_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, m_triCount * 3, GL_UNSIGNED_INT, 0, instanceCount));
}
