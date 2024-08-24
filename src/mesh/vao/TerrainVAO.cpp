#include "TerrainVAO.h"

#include "glm/glm.hpp"

#include "shader/Program.h"

#include "render/Batch.h"

namespace {
#pragma pack(push, 1)
    struct VertexEntry {
    };
#pragma pack(pop)
}

namespace mesh {
    kigl::GLVertexArray* TerrainVAO::prepare()
    {
        if (m_prepared) return m_vao.get();
        m_prepared = true;

        m_vao = std::make_unique<kigl::GLVertexArray>();
        m_vao->create("terrain");
        m_vbo.create();

        prepareVBO(m_vbo);
        prepareVAO(*m_vao, m_vbo);

        return m_vao.get();
    }

    void TerrainVAO::prepareVAO(
        kigl::GLVertexArray& vao,
        kigl::GLBuffer& vbo)
    {
        // NOTE KI nothing currently
    }

    void TerrainVAO::prepareVBO(kigl::GLBuffer& vbo)
    {
        // NOTE KI nothing currently
    }
}
