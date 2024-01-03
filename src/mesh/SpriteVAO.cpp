#include "SpriteVAO.h"

#include "glm/glm.hpp"

#include "asset/Program.h"

#include "render/Batch.h"

namespace {
#pragma pack(push, 1)
    struct VertexEntry {
    };
#pragma pack(pop)
}

kigl::GLVertexArray* SpriteVAO::prepare()
{
    if (m_prepared) return m_vao.get();
    m_prepared = true;

    m_vao = std::make_unique<kigl::GLVertexArray>();
    m_vao->create("sprite");
    m_vbo.create();

    prepareVBO(m_vbo);
    prepareVAO(*m_vao, m_vbo);

    return m_vao.get();
}

void SpriteVAO::prepareVAO(
    kigl::GLVertexArray& vao,
    kigl::GLBuffer& vbo)
{
    // NOTE KI nothing currently
}

void SpriteVAO::prepareVBO(kigl::GLBuffer& vbo)
{
    // NOTE KI nothing currently
}
