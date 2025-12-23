#include "SkinnedVAO.h"

#include <glm/glm.hpp>
#include <fmt/format.h>

#include "util/thread.h"

#include "shader/Shader.h"

#include "mesh/ModelMesh.h"

#include "VBO_impl.h"

namespace mesh {
    SkinnedVAO::SkinnedVAO(std::string_view name)
        : TexturedVAO{ name },
        m_jointVbo{ m_name + "_joint_vbo", ATTR_JOINT_INDEX, ATTR_JOINT_WEIGHT, VBO_JOINT_BINDING }
    {}

    SkinnedVAO::~SkinnedVAO() = default;

    void SkinnedVAO::prepareVAO()
    {
        TexturedVAO::prepareVAO();

        auto& vao = *m_vao;

        m_jointVbo.prepareVAO(vao);
    }

    void SkinnedVAO::clear()
    {
        TexturedVAO::clear();

        m_jointVbo.clear();
    }

    void SkinnedVAO::reserveVertexJoints(size_t count)
    {
        ASSERT_RT();

        m_jointVbo.reserveVertices(count);
    }

    void SkinnedVAO::updateVertexJoints(
        uint32_t baseVbo,
        std::span<animation::VertexJoint> vertexJoints)
    {
        ASSERT_RT();

        m_jointVbo.updateVertices(baseVbo, vertexJoints);
    }

    void SkinnedVAO::updateRT()
    {
        TexturedVAO::updateRT();

        m_jointVbo.updateVAO(*m_vao);
    }
}
