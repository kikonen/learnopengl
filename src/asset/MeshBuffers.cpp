#include "MeshBuffers.h"

#include <fmt/format.h>

#include "ki/GL.h"

MeshBuffers::MeshBuffers()
{
}

MeshBuffers::~MeshBuffers()
{
}

const std::string MeshBuffers::str() const
{
    return fmt::format(
        "<BUFFERS: vao={}, vbo={}, vbo_material={}, ebo={}>",
        VAO, VBO, VBO_MATERIAL, EBO);
}

void MeshBuffers::prepare(bool useVertex, bool useMaterial, bool useIndeces)
{
    if (m_prepared) return;
    m_prepared = true;

    VAO.create();
    if (useVertex) {
        VBO.create();
    }
    if (useMaterial) {
        VBO_MATERIAL.create();
    }
    if (useIndeces) {
        EBO.create();
    }
}
