#include "SpriteVAO.h"

#include "glm/glm.hpp"

#include "Shader.h"

#include "scene/Batch.h"

namespace {
#pragma pack(push, 1)
    struct VertexEntry {
    };
#pragma pack(pop)
}

GLVertexArray* SpriteVAO::prepare(Batch& batch)
{
    if (m_prepared) return m_vao.get();
    m_prepared = true;

    m_vao = std::make_unique<GLVertexArray>();
    m_vao->create();
    m_vbo.create();

    prepareVBO(m_vbo);
    prepareVAO(*m_vao, m_vbo);

    batch.prepareVAO(*m_vao, true);

    return m_vao.get();
}

void SpriteVAO::prepareVAO(
    GLVertexArray& vao,
    GLBuffer& vbo)
{
    // NOTE KI nothing currently
}

void SpriteVAO::prepareVBO(GLBuffer& vbo)
{
    // NOTE KI nothing currently
}
