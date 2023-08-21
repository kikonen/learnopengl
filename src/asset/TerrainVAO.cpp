#include "TerrainVAO.h"

#include "glm/glm.hpp"

#include "Program.h"

#include "render/Batch.h"

namespace {
#pragma pack(push, 1)
    struct VertexEntry {
    };
#pragma pack(pop)
}

GLVertexArray* TerrainVAO::prepare()
{
    if (m_prepared) return m_vao.get();
    m_prepared = true;

    m_vao = std::make_unique<GLVertexArray>();
    m_vao->create("terrain");
    m_vbo.create();

    prepareVBO(m_vbo);
    prepareVAO(*m_vao, m_vbo);

    return m_vao.get();
}

void TerrainVAO::prepareVAO(
    GLVertexArray& vao,
    GLBuffer& vbo)
{
    // NOTE KI nothing currently
}

void TerrainVAO::prepareVBO(GLBuffer& vbo)
{
    // NOTE KI nothing currently
}
