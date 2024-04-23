#include "SkinnedVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "asset/Shader.h"

#include "mesh/ModelMesh.h"

#include "VBO_impl.h"

namespace mesh {
    SkinnedVAO::SkinnedVAO(std::string_view name)
        : TexturedVAO{ name },
        m_boneVbo{ m_name + "_bone_vbo", ATTR_BONE_INDEX, ATTR_BONE_WEIGHT, VBO_BONE_BINDING }
    {}

    SkinnedVAO::~SkinnedVAO() = default;

    void SkinnedVAO::prepareVAO()
    {
        TexturedVAO::prepareVAO();

        auto& vao = *m_vao;

        m_boneVbo.prepareVAO(vao);
    }

    void SkinnedVAO::clear()
    {
        TexturedVAO::clear();

        m_boneVbo.clear();
    }

    const kigl::GLVertexArray* SkinnedVAO::registerModel(
        mesh::ModelMesh* mesh)
    {
        TexturedVAO::registerModel(mesh);

        m_boneVbo.addVertices(mesh->m_vertexBones);

        return m_vao.get();
    }

    void SkinnedVAO::updateRT()
    {
        TexturedVAO::updateRT();

        m_boneVbo.updateVAO(*m_vao);
    }
}
