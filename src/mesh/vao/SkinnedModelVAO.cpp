#include "SkinnedModelVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "asset/Shader.h"

#include "mesh/ModelMesh.h"

#include "VBO_impl.h"

namespace mesh {
    SkinnedModelVAO::SkinnedModelVAO(std::string_view name)
        : ModelVAO{ name },
        m_boneVbo{ m_name + "_bone_vbo", ATTR_BONE_INDEX, ATTR_BONE_WEIGHT, VBO_BONE_BINDING }
    {}

    SkinnedModelVAO::~SkinnedModelVAO() = default;

    void SkinnedModelVAO::prepareVAO()
    {
        ModelVAO::prepareVAO();

        auto& vao = *m_vao;

        m_boneVbo.prepareVAO(vao);
    }

    void SkinnedModelVAO::clear()
    {
        ModelVAO::clear();

        m_boneVbo.clear();
    }

    kigl::GLVertexArray* SkinnedModelVAO::registerModel(
        mesh::ModelMesh* mesh)
    {
        ModelVAO::registerModel(mesh);

        m_boneVbo.addVertices(mesh->m_vertexBones);

        return m_vao.get();
    }

    void SkinnedModelVAO::updateRT()
    {
        ModelVAO::updateRT();

        m_boneVbo.updateVAO(*m_vao);
    }
}
