#include "NonVaoMesh.h"

#include "mesh/LodMesh.h"

namespace mesh {
    NonVaoMesh::NonVaoMesh(std::string_view name)
        : Mesh{ name }
    {
        // NOTE KI matches OGLDEV: glDrawArrays(GL_TRIANGLES, 0, 6)
        // shader uses hardcoded vertex data + VERTEX_INDECES[gl_VertexID]
        m_vertexCount = 0;
        m_indexCount = 6;
    }

    NonVaoMesh::~NonVaoMesh() = default;

    const kigl::GLVertexArray* NonVaoMesh::setupVAO(
        mesh::TexturedVAO* vao,
        bool shared)
    {
        return nullptr;
    }

    void NonVaoMesh::prepareLodMesh(
        mesh::LodMesh& lodMesh)
    {
        lodMesh.m_baseVertex = getBaseVertex();
        lodMesh.m_baseIndex = getBaseIndex();
        lodMesh.m_indexCount = getIndexCount();

        auto& drawOptions = lodMesh.m_drawOptions;
        drawOptions.m_type = backend::DrawOptions::Type::arrays;
        drawOptions.m_mode = lodMesh.m_flags.tessellation
            ? backend::DrawOptions::Mode::patches
            : backend::DrawOptions::Mode::triangles;
    }

    AABB NonVaoMesh::calculateAABB(const glm::mat4& transform) const
    {
        return m_aabb;
    }
}
