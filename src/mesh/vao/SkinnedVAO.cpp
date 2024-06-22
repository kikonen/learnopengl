#include "SkinnedVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "util/thread.h"

#include "asset/Shader.h"

#include "mesh/ModelMesh.h"

#include "VBO_impl.h"

#include "animation/RigContainer.h"

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

    void SkinnedVAO::reserveVertexBones(size_t count)
    {
        ASSERT_RT();

        m_boneVbo.reserveVertices(count);
    }

    void SkinnedVAO::updateVertexBones(
        uint32_t baseVbo,
        std::span<animation::VertexBone> vertexBones)
    {
        ASSERT_RT();

        m_boneVbo.updateVertices(baseVbo, vertexBones);
    }

    void SkinnedVAO::updateRT()
    {
        TexturedVAO::updateRT();

        m_boneVbo.updateVAO(*m_vao);
    }
}
